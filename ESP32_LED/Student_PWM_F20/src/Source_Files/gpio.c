/**
 * @file 	gpio.c
 * @author 	Alexander Bork
 * @date 	9/13/2020
 * @brief	Contains function to initialize gpio functionality
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/*****************************************************************************
 * @brief
 * 		Sets up gpio ports for the LEDs, SI7021 temperature sensor, and LEUART routing.
 *
 * @details
 *		Enables the GPIO clock, and initializes the drive strengths for both LEDs.
 *		It uses defined parameters from brd_config.h and passes them into the GPIO_PinModeSet functions
 *		in order to set the mode and default states for both LEDs.
 *
 * @note
 *		Function is called by app_peripheral_setup in app.c.
 *
 ******************************************************************************/
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

	GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

	// Configure the temperature sensor pins
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_DRIVE_STRENGTH);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, SI7021_SENSOR_GPIOMODE, SI7021_DEFAULT);

	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, SI7021_SCL_DEFAULT);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, SI7021_SDA_DEFAULT);

	// LEUART - BLE configuration
	GPIO_DriveStrengthSet(LEUART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, true);

	GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModeInput, true);

}
