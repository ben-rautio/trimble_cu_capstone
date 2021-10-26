/**
 * @file	app.c
 * @author	Alexander Bork
 * @date	9/13/2020
 * @brief	Contains all the application driver functions
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include "stdio.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Static / Private Variables
//***********************************************************************************
//#define BLE_TEST_ENABLE

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *		Driver to initialize clocks, peripherals, and scheduler for usage.
 *
 * @details
 *		This function just contains calls to functions in cmu.c, gpio.c, and letimer.c, SI7021.c
 *		and ble.c that initialize PWM functionality from Letimer0, and enable routing from the letimer0 clock
 *		to the gpio ports linked to the LEDs.
 *
 *
 * @note
 *		Is called once from main.c to setup any peripheral/clock connections for use
 *		in PWM functionality.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	scheduler_open();
	sleep_open();
	si7021_i2c_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
	sleep_block_mode(SYSTEM_BLOCK_EM);
	ble_open(LEUART_TX_CB, LEUART_RX_CB);
	add_scheduled_event(BOOT_UP_CB);
}




/**************************s*************************************************//**
 * @brief
 *		Defines parameters in a letimer struct
 *
 * @details
 *		Is called by app_peripheral_setup to initialize values within the application LETIMER struct.
 *		This function passes the struct to letimer_pwn_open to set parameters for PWM operation.
 *		After it sets the parameters for the LETIMER's PWM operation, it calls letimer_start to start the clock.
 *
 * @note
 *		None of the parameters set within the app_letimer_pwm_typedef struct affect the letimer
 *		until letimer_pwm_open is called.
 *
 * @param[in] period
 *		A constant that defines the total period of the PWM pulse
 *
 * @param[in] act_period
 *		A constant that defines the duty cycle of the PWM pulse
 *
 * @param[in] out0_route
 *		The base peripheral address of the first route for the LETIMER0.
 *		out0_route is the address of LED0
 *
 * @param[in] out1_route
 *		The base peripheral address of the second route for the LETIMER0.
 *		out1_route is the address of LED1
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef letimer_pwm_struct;


	letimer_pwm_struct.debugRun = false;
	letimer_pwm_struct.enable = false;
	letimer_pwm_struct.out_pin_0_en = false;
	letimer_pwm_struct.out_pin_1_en = false;
	letimer_pwm_struct.out_pin_route0 = out0_route;
	letimer_pwm_struct.out_pin_route1 = out1_route;
	letimer_pwm_struct.period = period;
	letimer_pwm_struct.active_period = act_period;
	letimer_pwm_struct.uf_irq_enable = true;
	letimer_pwm_struct.comp0_irq_enable = false;
	letimer_pwm_struct.comp1_irq_enable = false;
	letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
	letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
	letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;

	letimer_pwm_open(LETIMER0, &letimer_pwm_struct);

	// letimer_start will inform the LETIMER0 peripheral to begin counting.
}

/***************************************************************************//**
 * @brief
 *		Callback function for the UnderFlow interrupt
 *
 * @details
 *		Removes the scheduled event from the event_scheduled static variable
 *		in scheduler.c. Every time the function is called, it unblocks the mode
 *		it is currently in and blocks the next lowest Energy Mode.
 *
 * @note
 *		Cycles through blocking different EM states, resets the EM state to EM0
 *		before getting to EM4.
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_CB);
	remove_scheduled_event(LETIMER0_UF_CB);
	si7021_i2c_read(SI7021_READ_CB);
}

/***************************************************************************//**
 * @brief
 *		Callback function for the Comp0 interrupt
 *
 * @details
 *		Removes the scheduled event from the event_scheduled static variable
 *		in scheduler.c
 *
 *
 * @note
 *		The full callback function has not yet been integrated with the
 *		sleep_routine functionality.
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void){
	remove_scheduled_event(LETIMER0_COMP0_CB);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *		Callback function for the Comp1 interrupt
 *
 * @details
 *		Removes the scheduled event from the event_scheduled static variable
 *		in scheduler.c
 *
 * @note
 *		The full callback function has not yet been integrated with the
 *		sleep_routine functionality.
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void){
	remove_scheduled_event(LETIMER0_COMP1_CB);
	EFM_ASSERT(false);
}



/***************************************************************************//**
 * @brief
 *		Callback function for reading the temperature from the SI7021.
 *
 * @details
 *		Removes the SI7021_READ event from the event_scheduled static variable
 *		in scheduler.c. Then reads the stored temperature from the
 *		si7021_temp_convert() function. If the read temperature is greater than
 *		80.0 *F, then turn on LED1 until the temperature is read to be lower
 *		during a future Letimer_UF interrupt. It also sends a string to ble_write
 *		to be sent to the bluetooth module.
 *
 * @note
 *		The LED output is no longer dependent on the LETIMER.
 *
 ******************************************************************************/
void scheduled_si7021_read_cb(void){
	remove_scheduled_event(SI7021_READ_CB);
	float tmp = si7021_temp_convert();
	char tempStr[40];
	int success = sprintf(tempStr, "\nTemp = %4.1f F\n", (double) tmp);
	EFM_ASSERT(!(success == 0));
	ble_write(tempStr);
	if(tmp >= 80.0){
		GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else{
		GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
}

/***************************************************************************//**
 * @brief
 *		Callback function for booting up the PG12 board.
 *
 * @details
 *		Removes the BOOT_UP_CB event from the event_scheduled static variable
 *		in scheduler.c. Then writes "Hello World" to the bluetooth module. Then it
 *		starts the letimer after writing to the bluetooth module.
 *
 * @note
 *		Ble_test will only be called to change the bluetooth module's name
 *		if BLE_TEST_ENABLE is defined at the top of app.c.
 *
 ******************************************************************************/
void scheduled_boot_up_cb(void){
	remove_scheduled_event(BOOT_UP_CB);
#ifdef BLE_TEST_ENABLE
	bool ble = ble_test("ABorkMod");
	EFM_ASSERT(ble);
	timer_delay(2000);
#endif
	ble_write("Hello World\n");
	letimer_start(LETIMER0, true);
}

/***************************************************************************//**
 * @brief
 *		Callback function for LEUART TX done event.
 *
 * @details
 *		Removes the LEUART_TX_CB event from the event_scheduled static variable
 *		in scheduler.c.
 *
 * @note
 *		Currently has no functionality.
 *
 ******************************************************************************/
void scheduled_leuartTX_cb(void){
	remove_scheduled_event(LEUART_TX_CB);
}








