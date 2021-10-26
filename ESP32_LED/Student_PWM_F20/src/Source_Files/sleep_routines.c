/**************************************************************************
* @file sleep_routines.c
***************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
***************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/

/**
 * @file 	sleep_routines.c
 * @author 	Silicon Labs
 * @date 	9/27/2020
 * @brief 	Contains functions for processing sleep states. Copyrighted by Silicon Labs, implemented by Alexander Bork.
 *
 */




//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"

//***********************************************************************************
// Private variables
//***********************************************************************************

static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// Private functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *		Initialized the blocks on all Energy Modes
 *
 * @details
 *		Sets the amount of blocks on all Energy Modes to 0.
 *
 * @note
 *		INIT_EM_MODE is defined as 0.
 *
 ******************************************************************************/
void sleep_open(void){
	lowest_energy_mode[EM0] = INIT_EM_MODE;
	lowest_energy_mode[EM1] = INIT_EM_MODE;
	lowest_energy_mode[EM2] = INIT_EM_MODE;
	lowest_energy_mode[EM3] = INIT_EM_MODE;
	lowest_energy_mode[EM4] = INIT_EM_MODE;
}



/***************************************************************************//**
 * @brief
 *		Adds a block to a given Energy Mode.
 *
 * @details
 * 		Increases the amount of blocks on any given Energy Mode 0-4 by 1.
 *
 * @note
 *		Uses CORE statements to have the process atomic; i.e. un-interruptible.
 *
 * @param[in] EM
 *		The Energy Mode to have a block set on.
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		lowest_energy_mode[EM]++;
		CORE_EXIT_CRITICAL();
}



/***************************************************************************//**
 * @brief
 *		Removes a block from a given Energy Mode.
 *
 * @details
 *		Decreases the amount of blocks on any given Energy Mode 0-4 by 1.
 *		EFM_ASSERT statements are used to make sure that no more than 5 blocks
 *		and no less than 0 blocks on the Energy Mode.
 *
 * @note
 *		Will only remove a block if there is at least one block on the Energy Mode
 *		at the time of the function call.
 *		Uses CORE statements to have the process atomic; i.e. un-interruptible.
 *
 * @param[in] EM
 *		The Energy Mode to have a block removed from.
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
	if(lowest_energy_mode[EM] > 0){
		CORE_DECLARE_IRQ_STATE;
		CORE_ENTER_CRITICAL();
		lowest_energy_mode[EM]--;
		CORE_EXIT_CRITICAL();
	}
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
	EFM_ASSERT(lowest_energy_mode[EM] < 5);
}



/***************************************************************************//**
 * @brief
 *		Enters the lowest sleep state allows by blocks.
 *
 * @details
 *		This uses if and else if statements to find the lowest sleep state without
 *		any blocks on it. If a block is found on a sleep state, EM will enter the
 *		next highest Energy Mode.
 *
 * @note
 *		Uses CORE statements to have the process atomic; i.e. un-interruptible.
 *
 ******************************************************************************/
void enter_sleep(void){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	if(lowest_energy_mode[EM0] > 0){
		CORE_EXIT_CRITICAL();
		return;
	}
	else if(lowest_energy_mode[EM1] > 0){
		CORE_EXIT_CRITICAL();
		return;
	}
	else if(lowest_energy_mode[EM2] > 0){
		EMU_EnterEM1();
	}
	else if(lowest_energy_mode[EM3] > 0){
		EMU_EnterEM2(true);
	}
	else{
		EMU_EnterEM3(true);
	}
	CORE_EXIT_CRITICAL();
	return;
}



/***************************************************************************//**
 * @brief
 *		Returns the index of the lowest block Energy Mode.
 *
 * @details
 *		Uses a while loop to look at the number of blocks on each Energy Mode.
 *		If the number of blocks on a mode is not 0, then the function will return
 *		that energy mode.
 *
 * @note
 *		i will return the Energy Mode directly below the Energy Mode currently
 *		being used.
 *
 * @return
 *		The index of the lowest blocked Energy Mode.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
	uint32_t i = 0;
	while(i < MAX_ENERGY_MODES){
		if(lowest_energy_mode[i] != 0){
			return i;
		}
		i++;
	}
	return MAX_ENERGY_MODES - 1;
}

