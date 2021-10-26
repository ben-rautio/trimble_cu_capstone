/**
 * @file 	cmu.c
 * @author 	Alexander Bork
 * @date 	9/13/2020
 * @brief 	Contains a function to initialize CMU functionality
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

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
 * 		Disables and Enables certain clocks for use
 *
 * @details
 *		Enables the High Frequency peripheral clock, enables the LFXO oscillator, and disables the LFRCO oscillator.
 *		Then it routes the low frequency clock to LETIMER0 clock tree, and routes the LFXO oscillator to the LEUART0 clock tree.
 *		Finally it enables the global LowF low energy clock.
 *
 * @note
 *		Function is called by app_peripheral_setup in app.c to allow synchronous behavior in the project.
 *
 ******************************************************************************/
void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		// By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
		// Disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);	 // What is the enumeration required for LFRCO?

		// Enable the Low Frequency Crystal Oscillator, LFXO
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);	// What is the enumeration required for LFXO?

		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		// Route LF clock to LETIMER0 clock tree
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);	// What clock tree does the LETIMER0 reside on?

		// Route LFX oscillator to LEUART0 clock tree
		CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

		// Now, you must ensure that the global Low Frequency is enabled
		CMU_ClockEnable(cmuClock_CORELE, true);	//This enumeration is found in the Lab 2 assignment

		//CMU_ClockEnable(cmuClock_I2C0, true);
}
