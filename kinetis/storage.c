/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Paul Brook
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <string.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "lib/fatfs/ff.h"
#include "extmod/fsusermount.h"

#include "flash.h"
#include "storage.h"

STATIC byte flash_cache_mem[P_SECTOR_SIZE] __attribute__((aligned(4)));
extern uint8_t _flash_fs_start[];
extern uint8_t _flash_fs_end[];
#define FLASH_FS_SIZE (_flash_fs_end - _flash_fs_start)
#define FLASH_PART1_START_BLOCK 0x100
#define FLASH_PART1_NUM_BLOCKS (FLASH_FS_SIZE / FLASH_BLOCK_SIZE)

#define FLASH_FLAG_DIRTY        (1)
#define FLASH_FLAG_FORCE_WRITE  (2)
#define FLASH_FLAG_ERASED       (4)
static bool flash_is_initialised = false;
static bool flash_cache_dirty;
static uint32_t flash_cache_sector_id = -1;
static uint32_t flash_tick_counter_last_write;

static void flash_cache_flush(void) {
    uint8_t *dest;
    if (!flash_cache_dirty) {
        return;
    }
    dest = _flash_fs_start + flash_cache_sector_id * P_SECTOR_SIZE;
    flash_write(dest, flash_cache_mem);
}

void storage_init(void) {
    if (!flash_is_initialised) {
        flash_cache_dirty = false;
        flash_cache_sector_id = -1;
        flash_tick_counter_last_write = 0;
        flash_is_initialised = true;
        flash_init();
    }
}

uint32_t storage_get_block_size(void) {
    return FLASH_BLOCK_SIZE;
}

uint32_t storage_get_block_count(void) {
    return FLASH_PART1_START_BLOCK + FLASH_PART1_NUM_BLOCKS;
}

void storage_flush(void) {
    flash_cache_flush();
}

static void build_partition(uint8_t *buf, int boot, int type, uint32_t start_block, uint32_t num_blocks) {
    buf[0] = boot;

    if (num_blocks == 0) {
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
    } else {
        buf[1] = 0xff;
        buf[2] = 0xff;
        buf[3] = 0xff;
    }

    buf[4] = type;

    if (num_blocks == 0) {
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    } else {
        buf[5] = 0xff;
        buf[6] = 0xff;
        buf[7] = 0xff;
    }

    buf[8] = start_block;
    buf[9] = start_block >> 8;
    buf[10] = start_block >> 16;
    buf[11] = start_block >> 24;

    buf[12] = num_blocks;
    buf[13] = num_blocks >> 8;
    buf[14] = num_blocks >> 16;
    buf[15] = num_blocks >> 24;
}

unsigned bsum(const uint8_t *data)
{
    unsigned sum=0;
    int i;
    for (i = 0; i < FLASH_BLOCK_SIZE; i++) {
        sum += *(data++);
    }
    return sum;
}

bool storage_read_block(uint8_t *dest, uint32_t block) {
    //DEBUG_printf("RD %u\n", block);
    if (block == 0) {
        // fake the MBR so we can decide on our own partition table

        for (int i = 0; i < 446; i++) {
            dest[i] = 0;
        }

        build_partition(dest + 446, 0, 0x01 /* FAT12 */, FLASH_PART1_START_BLOCK, FLASH_PART1_NUM_BLOCKS);
        build_partition(dest + 462, 0, 0, 0, 0);
        build_partition(dest + 478, 0, 0, 0, 0);
        build_partition(dest + 494, 0, 0, 0, 0);

        dest[510] = 0x55;
        dest[511] = 0xaa;

        return true;

    } else {
        // non-MBR block, get data from flash memory, possibly via cache
        int sector;
        unsigned long offset;
        uint8_t *src;
        block -= FLASH_PART1_START_BLOCK;
        offset = block * FLASH_BLOCK_SIZE;
        if (offset > FLASH_FS_SIZE) {
            return false;
        }
        sector = offset / P_SECTOR_SIZE;
        if (sector == flash_cache_sector_id) {
            src = flash_cache_mem + (offset % P_SECTOR_SIZE);
        } else {
            src = _flash_fs_start + offset;
        }

        memcpy(dest, src, FLASH_BLOCK_SIZE);
        return true;
    }
}

bool storage_write_block(const uint8_t *src, uint32_t block) {
    //DEBUG_printf("WR %u\n", block);
    if (block == 0) {
        // can't write MBR, but pretend we did
        return true;

    } else {
        // non-MBR block, copy to cache
        int sector;
        unsigned long offset;
        uint8_t *dest;

        block -= FLASH_PART1_START_BLOCK;
        offset = block * FLASH_BLOCK_SIZE;
        if (offset > FLASH_FS_SIZE) {
            return false;
        }
        sector = offset / P_SECTOR_SIZE;
        if (sector != flash_cache_sector_id) {
            flash_cache_flush();
            flash_cache_sector_id = sector;
            memcpy(flash_cache_mem, _flash_fs_start + sector * P_SECTOR_SIZE, P_SECTOR_SIZE);
        }
        dest = flash_cache_mem + (offset % P_SECTOR_SIZE);
        if (memcmp(dest, src, FLASH_BLOCK_SIZE) != 0) {
            flash_cache_dirty = true;
            memcpy(flash_cache_mem + (offset % P_SECTOR_SIZE), src, FLASH_BLOCK_SIZE);
        }
        return true;
    }
}

mp_uint_t storage_read_blocks(uint8_t *dest, uint32_t block_num, uint32_t num_blocks) {
    for (size_t i = 0; i < num_blocks; i++) {
        if (!storage_read_block(dest + i * FLASH_BLOCK_SIZE, block_num + i)) {
            return 1; // error
        }
    }
    return 0; // success
}

mp_uint_t storage_write_blocks(const uint8_t *src, uint32_t block_num, uint32_t num_blocks) {
    for (size_t i = 0; i < num_blocks; i++) {
        if (!storage_write_block(src + i * FLASH_BLOCK_SIZE, block_num + i)) {
            return 1; // error
        }
    }
    return 0; // success
}

/******************************************************************************/
// MicroPython bindings
//
// Expose the flash as an object with the block protocol.

// there is a singleton Flash object
STATIC const mp_obj_base_t pyb_flash_obj = {&pyb_flash_type};

STATIC mp_obj_t pyb_flash_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return (mp_obj_t)&pyb_flash_obj;
}

STATIC mp_obj_t pyb_flash_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    mp_uint_t ret = storage_read_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_flash_readblocks_obj, pyb_flash_readblocks);

STATIC mp_obj_t pyb_flash_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    mp_uint_t ret = storage_write_blocks(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / FLASH_BLOCK_SIZE);
    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_flash_writeblocks_obj, pyb_flash_writeblocks);

STATIC mp_obj_t pyb_flash_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case BP_IOCTL_INIT: storage_init(); return MP_OBJ_NEW_SMALL_INT(0);
        case BP_IOCTL_DEINIT: storage_flush(); return MP_OBJ_NEW_SMALL_INT(0); // TODO properly
        case BP_IOCTL_SYNC: storage_flush(); return MP_OBJ_NEW_SMALL_INT(0);
        case BP_IOCTL_SEC_COUNT: return MP_OBJ_NEW_SMALL_INT(storage_get_block_count());
        case BP_IOCTL_SEC_SIZE: return MP_OBJ_NEW_SMALL_INT(storage_get_block_size());
        default: return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_flash_ioctl_obj, pyb_flash_ioctl);

STATIC const mp_map_elem_t pyb_flash_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_readblocks), (mp_obj_t)&pyb_flash_readblocks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_writeblocks), (mp_obj_t)&pyb_flash_writeblocks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ioctl), (mp_obj_t)&pyb_flash_ioctl_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_flash_locals_dict, pyb_flash_locals_dict_table);

const mp_obj_type_t pyb_flash_type = {
    { &mp_type_type },
    .name = MP_QSTR_Flash,
    .make_new = pyb_flash_make_new,
    .locals_dict = (mp_obj_t)&pyb_flash_locals_dict,
};

void pyb_flash_init_vfs(fs_user_mount_t *vfs) {
    vfs->flags |= FSUSER_NATIVE | FSUSER_HAVE_IOCTL;
    vfs->readblocks[0] = (mp_obj_t)&pyb_flash_readblocks_obj;
    vfs->readblocks[1] = (mp_obj_t)&pyb_flash_obj;
    vfs->readblocks[2] = (mp_obj_t)storage_read_blocks; // native version
    vfs->writeblocks[0] = (mp_obj_t)&pyb_flash_writeblocks_obj;
    vfs->writeblocks[1] = (mp_obj_t)&pyb_flash_obj;
    vfs->writeblocks[2] = (mp_obj_t)storage_write_blocks; // native version
    vfs->u.ioctl[0] = (mp_obj_t)&pyb_flash_ioctl_obj;
    vfs->u.ioctl[1] = (mp_obj_t)&pyb_flash_obj;
}
