#include "board.h"
#include "flash.h"
#include "fsl_debug_console.h"

pFLASHCOMMANDSEQUENCE g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)0xFFFFFFFF;

// Array to copy __Launch_Command func to RAM.
uint16_t ramFunc[LAUNCH_CMD_SIZE/2];

// Flash Standard Software Driver Structure.
FLASH_SSD_CONFIG flashSSDConfig =
{
    FTFx_REG_BASE,          /*! FTFx control register base */
    P_FLASH_BASE,           /*! Base address of PFlash block */
    P_FLASH_SIZE,           /*! Size of PFlash block */
    FLEXNVM_BASE,           /*! Base address of DFlash block */
    0,                      /*! Size of DFlash block */
    EERAM_BASE,             /*! Base address of EERAM block */
    0,                      /*! Size of EEE block */
    DEBUGENABLE,            /*! Background debug mode enable bit */
    NULL_CALLBACK           /*! Pointer to callback function */
};

void flash_init(void)
{
    FlashInit(&flashSSDConfig);

    g_FlashLaunchCommand = (pFLASHCOMMANDSEQUENCE)RelocateFunction((uint32_t)ramFunc , LAUNCH_CMD_SIZE ,(uint32_t)FlashCommandSequence);
}

void flash_write(uint8_t *dest, uint8_t *src)
{
    uint32_t result;

    result = FlashEraseSector(&flashSSDConfig, (long)dest, P_SECTOR_SIZE, g_FlashLaunchCommand);
    if (FTFx_OK != result)
    {
        // TODO: Error reporting
        return;
    }

    result = FlashProgram(&flashSSDConfig, (long)dest, P_SECTOR_SIZE, \
                                   src, g_FlashLaunchCommand);
    if (FTFx_OK != result)
    {
        // TODO: Error reporting
        return;
    }

}
