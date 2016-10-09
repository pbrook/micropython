#include "py/runtime.h"
#include "board.h"
#include "fsl_clock_manager.h"
#include "fsl_os_abstraction.h"
#include "fsl_dspi_master_driver.h"
#include "cube_spi.h"

#define DSPI_MASTER_INSTANCE        0
#define TRANSFER_BAUDRATE           (500000U)           /*! Transfer baudrate - 500k */
#define MASTER_TRANSFER_TIMEOUT     (5000U)             /*! Transfer timeout of master - 5s */

int spi_init(void)
{
    uint32_t calculatedBaudRate;

    dspi_status_t dspiResult;
    dspi_master_state_t masterState;
    dspi_device_t masterDevice;
    dspi_master_user_config_t masterUserConfig = {
        .isChipSelectContinuous     = false,
        .isSckContinuous            = false,
        .pcsPolarity                = kDspiPcs_ActiveLow,
        .whichCtar                  = kDspiCtar0,
        .whichPcs                   = kDspiPcs0
    };

    // Setup the configuration.
    masterDevice.dataBusConfig.bitsPerFrame = 8;
    masterDevice.dataBusConfig.clkPhase     = kDspiClockPhase_FirstEdge;
    masterDevice.dataBusConfig.clkPolarity  = kDspiClockPolarity_ActiveHigh;
    masterDevice.dataBusConfig.direction    = kDspiMsbFirst;

    // Initialize master driver.
    dspiResult = DSPI_DRV_MasterInit(DSPI_MASTER_INSTANCE,
                                     &masterState,
                                     &masterUserConfig);
    if (dspiResult != kStatus_DSPI_Success)
    {
        DEBUG_printf("\r\nERROR: Can not initialize master driver \r\n");
        return -1;
    }

    // Configure baudrate.
    masterDevice.bitsPerSec = TRANSFER_BAUDRATE;
    dspiResult = DSPI_DRV_MasterConfigureBus(DSPI_MASTER_INSTANCE,
                                             &masterDevice,
                                             &calculatedBaudRate);
    if (dspiResult != kStatus_DSPI_Success)
    {
        DEBUG_printf("\r\nERROR: failure in configuration bus\r\n");
        return -1;
    }
    else
    {
        DEBUG_printf("\r\n Transfer at baudrate %lu \r\n", calculatedBaudRate);
    }

    return 0;
}

void send_data(const uint8_t *buf, int len)
{
    dspi_status_t dspiResult;

    // Send the data.
    dspiResult = DSPI_DRV_MasterTransferBlocking(DSPI_MASTER_INSTANCE,
                                                 NULL,
                                                 buf,
                                                 NULL,
                                                 len,
                                                 MASTER_TRANSFER_TIMEOUT);
    if (dspiResult != kStatus_DSPI_Success)
    {
        DEBUG_printf("\r\nERROR: send data error \r\n");
    }
}
