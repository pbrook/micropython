/*
 * This file is part of the MicroPython project, http://micropython.org/
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

#include <assert.h>
#include <string.h>

#include "py/runtime.h"
#include "gpio_pins.h"
#include "fsl_dspi_master_driver.h"
#include "cube_spi.h"
#include "board.h"

enum {
    cp_blank = GPIO_MAKE_PIN(GPIOA_IDX, 2u), // D5/PTA2
    cp_xlat = GPIO_MAKE_PIN(GPIOB_IDX, 23u), // D4/PTB23
    cp_mode = GPIO_MAKE_PIN(GPIOA_IDX, 1u), // D3/PTA1
    cp_addr0 = GPIO_MAKE_PIN(GPIOC_IDX, 10u), // A5/PTC10
    cp_addr1 = GPIO_MAKE_PIN(GPIOC_IDX, 11u), // A4/PTC11
    cp_addr2 = GPIO_MAKE_PIN(GPIOB_IDX, 11u), // A3/PTB11
    cp_addr3 = GPIO_MAKE_PIN(GPIOB_IDX, 10u), // A2/PTB10
    cp_addr3in = GPIO_MAKE_PIN(GPIOB_IDX, 3u), // A1/PTB3
    cp_addr_en = GPIO_MAKE_PIN(GPIOB_IDX, 2u), // A0/PTB2
    cp_gsclk = GPIO_MAKE_PIN(GPIOC_IDX, 2u), // D6/PTC2 alt4 FTM0 CH1
};

#define PINCFG(name, val) \
  { \
    .pinName = cp_##name, \
    .config.outputLogic = val, \
    .config.slewRate = kPortSlowSlewRate, \
    .config.isOpenDrainEnabled = false, \
    .config.driveStrength = kPortLowDriveStrength, \
  }

const gpio_output_pin_user_config_t cubePins[] = {
    PINCFG(blank, 1),
    PINCFG(xlat, 0),
    PINCFG(mode, 0),
    PINCFG(addr0, 0),
    PINCFG(addr1, 0),
    PINCFG(addr2, 0),
    PINCFG(addr3, 0),
    PINCFG(addr3in, 1),
    PINCFG(addr_en, 1),
    PINCFG(gsclk, 0),
  {
    .pinName = GPIO_PINS_OUT_OF_RANGE,
  }
};

STATIC mp_obj_t mod_ucube_init(void) {
    const gpio_output_pin_user_config_t *cfg;

    for (cfg = cubePins; cfg->pinName != GPIO_PINS_OUT_OF_RANGE; cfg++) {
        GPIO_DRV_OutputPinInit(cfg);
    }

    spi_init();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_ucube_init_obj, mod_ucube_init);

STATIC mp_obj_t mod_ucube_putpixel(mp_obj_t pixel_in, mp_obj_t color_in) {
    mp_uint_t pixel = mp_obj_get_int_truncated(pixel_in);
    mp_uint_t color = mp_obj_get_int_truncated(color_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mod_ucube_putpixel_obj, mod_ucube_putpixel);


STATIC const mp_rom_map_elem_t mp_module_ucube_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_ucube) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mod_ucube_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_putpixel), MP_ROM_PTR(&mod_ucube_putpixel_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_ucube_globals, mp_module_ucube_globals_table);

const mp_obj_module_t mp_module_ucube = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_ucube_globals,
};
