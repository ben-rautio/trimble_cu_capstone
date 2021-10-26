/**
 * @file 	leuart.c
 * @author 	Alexander Bork
 * @date 	10/30/2020
 * @brief 	Contains leuart driver functions.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>
#include <stdio.h>
//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;
#define		RAND_SF			0x1
#define		SF_CLEAR		0x0

static LEUART_SM 	LE_SM;


/***************************************************************************//**
 * @brief LEUART driver
 *
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void leuart_txcIRPT(void);
static void leuart_txblIRPT(void);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *		Initializes the LEUART peripheral for functionality.
 *
 * @details
 *		Takes in pointers to leuart and leuart_settings struct to initialize the
 *		leuart peripheral. Enables the clock to the leuart peripheral and passes the
 *		values from leuart_settings to the leuart peripheral.
 *
 * @note
 *		Due to the amount of asynchronous registers of the LEUART, while syncbusy
 *		loops are utilized to ensure a portion of the leuart is configured before
 *		configuring the next portion.
 *		Does not enable any interrupts, and only enables the LEUART NVIC at the
 *		end of the function.
 *
 * @param[in] *leuart
 * 		A pointer to the base addres of the LEUART peripheral being used.
 *
 * @param[in] *leuart_settings
 * 		A pointer to the base address of the settings struct for the LEUART
 * 		functionality.
 *
 ******************************************************************************/
void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	if(leuart == LEUART0){
		CMU_ClockEnable(cmuClock_LEUART0, true);
	}

	leuart->STARTFRAME = RAND_SF;
	while(leuart->SYNCBUSY);
	EFM_ASSERT(leuart->STARTFRAME == RAND_SF);
	leuart->STARTFRAME = SF_CLEAR;
	while(leuart->SYNCBUSY);

	LE_SM.tx_evt = leuart_settings->tx_done_evt;
	LE_SM.rx_evt = leuart_settings->rx_done_evt;

	LEUART_Init_TypeDef leuart_initTD;

	leuart_initTD.baudrate = leuart_settings->baudrate;
	leuart_initTD.enable = false;
	leuart_initTD.refFreq = leuart_settings->refFreq;
	leuart_initTD.databits = leuart_settings->databits;
	leuart_initTD.parity = leuart_settings->parity;
	leuart_initTD.stopbits = leuart_settings->stopbits;

	while(leuart->SYNCBUSY);
	LEUART_Reset(leuart);
	LEUART_Init(leuart, &leuart_initTD);	// Initialize LEUART0

	while(leuart->SYNCBUSY);	// Wait for leuart to sync

	// Route the LEUART to the header pins and enable the routing
	leuart->ROUTEPEN = leuart_settings->tx_pin_en | leuart_settings->rx_pin_en ;
	leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc;

	// Verify that RX and TX are disabled
	//EFM_ASSERT(!(leuart->STATUS & (LEUART_STATUS_RXENS | LEUART_STATUS_TXENS)));

	while(leuart->SYNCBUSY);
	leuart->CMD = LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;
	while(leuart->SYNCBUSY);

	// Enable RX and TX, and verify that they are enabled
	LEUART_Enable(leuart, leuart_settings->enable);

//	while(leuart->SYNCBUSY);
//	leuart->CMD = LEUART_CMD_CLEARTX | LEUART_CMD_CLEARRX;
	while(leuart->SYNCBUSY);

	leuart->IFC = leuart->IF;
	while(leuart->SYNCBUSY);
	if((leuart->STATUS & LEUART_STATUS_TXENS) == false){
		while((leuart->STATUS & LEUART_STATUS_TXENS)== false);
	}
	if((leuart->STATUS & LEUART_STATUS_RXENS) == false){
		while((leuart->STATUS & LEUART_STATUS_RXENS)== false);
	}
	EFM_ASSERT(leuart->STATUS & LEUART_STATUS_TXENS);
	EFM_ASSERT(leuart->STATUS & LEUART_STATUS_RXENS);
	NVIC_EnableIRQ(LEUART0_IRQn);
}

/***************************************************************************//**
 * @brief
 * 		The TXC handler for the LEUART state machine.
 *
 * @details
 *		The switch statement in the function will only have functionality for the
 *		DONE state. In this state, the TXC interrupt will unblock EM3 sleep mode,
 *		add the scheduled event to the scheduler, and reset the state machine.
 *
 * @note
 *  	If a TXC interrupt occurs in any state besides the DONE state, then
 *  	an EFM_ASSERT statement will be asserted to false; indicating a false
 *  	state.
 *
 ******************************************************************************/
void leuart_txcIRPT(void){
	switch(LE_SM.states){
		case START:
			EFM_ASSERT(false);
			break;
		case SEND_STR:
			EFM_ASSERT(false);
			break;
		case DONE:
			sleep_unblock_mode(LEUART_TX_EM);
			LE_SM.idx = 0;
			LE_SM.busy = false;
			LE_SM.leuart->IEN &= ~LEUART_IEN_TXC;
			add_scheduled_event(LE_SM.tx_evt);
			LE_SM.states = START;
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}


/***************************************************************************//**
 * @brief
 * 		The TXBL handler for the LEUART state machine.
 *
 * @details
 *		The switch statement in the function will either send bytes through
 *		the leuart's transmit register or switch the state of the state
 *		machine depending on when the TXBL interrupt occurs.
 *
 * @note
 *  	If a TXBL interrupt occurs in the DONE or default states, then an
 *  	EFM_ASSERT will be asserted as false.
 *
 ******************************************************************************/
void leuart_txblIRPT(void){
	switch(LE_SM.states){
		case START:
			LE_SM.states = SEND_STR;
			break;
		case SEND_STR:
			if(LE_SM.idx == (LE_SM.strLen - 1)){
				LE_SM.states = DONE;
				LE_SM.leuart->IEN &= ~LEUART_IEN_TXBL;
				leuart_app_transmit_byte(LE_SM.leuart, (uint8_t) LE_SM.message[LE_SM.idx]);
				LE_SM.leuart->IEN |= LEUART_IEN_TXC;
				break;
			}
			else{
				while(leuart_tx_busy(LE_SM.leuart));
				LE_SM.idx += 1;
				leuart_app_transmit_byte(LE_SM.leuart, LE_SM.message[LE_SM.idx - 1]);
				break;
			}
		case DONE:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *		Interrupt Handler for LEUART0.
 *
 * @details
 *		A variable int_flag will store any interrupt flag if it has also been enabled.
 *		All of the interrupts will then be cleared from LEUART0->IF. The
 *		function handles the scheduling of processing each interrupt.
 *
 * @note
 *		Int_flag will be used to service every interrupt, with the priority given
 *		to txbl, then txc.
 *
 ******************************************************************************/
void LEUART0_IRQHandler(void){
	uint32_t int_flag;
	int_flag = LEUART0->IF & LEUART0->IEN;
	LEUART0->IFC = int_flag;
	if(int_flag & LEUART0_TXBL_IRPT){	// Handling for sending a new character
		leuart_txblIRPT();
	}
	if(int_flag & LEUART0_TXC_IRPT){	//Handling for ending the transmission
		leuart_txcIRPT();
	}
}

/**************************s*************************************************//**
 * @brief
 *		Initializes the LEUART0 State Machine for bluetooth transmission.
 *
 * @details
 *		After initializing the State Machine, enable the txbl interrupt and exit
 *		the critical state. This will drive the board into the LEUART0 interrupt
 *		handler and start sending each byte of the string.
 *
 * @note
 *		This function is made atomic by CORE CRITICAL calls.
 *
 * @param[in] *leuart
 *		A pointer to the base address of one of the leuart peripherals.
 *
 * @param[in] *string
 *		A pointer to the string being sent to the bluetooth device.
 *
 * @param[in] string_len
 *		The length of the string being sent.
 *
 ******************************************************************************/
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	sleep_block_mode(LEUART_TX_EM);
	LE_SM.leuart = leuart;
	LE_SM.busy = true;
	LE_SM.states = START;
	LE_SM.strLen = string_len;
	LE_SM.idx = 0;
	strcpy(LE_SM.message, string);

	LE_SM.leuart->IEN = LEUART_IEN_TXBL;
	CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *		Checks the state of the LEUART transmit buffer
 *
 * @details
 *		The function accesses the status register of the LEUART to check if the
 *		transmit buffer has data in it.
 * @note
 *		The function will return true if there is currently data in the transmit
 *		buffer.
 *
 * @param[in] *leuart
 *		A pointer to the base address of the leuart peripheral.
 *
 * @return
 * 		A boolean stating if the transmit buffer is busy.
 ******************************************************************************/
bool leuart_tx_busy(LEUART_TypeDef *leuart){
	bool busy = false;
	if(leuart->STATUS == LEUART_STATUS_TXBL){
		busy = true;
	}
	return busy;
}


/***************************************************************************//**
 * @brief
 *   	LEUART STATUS function returns the STATUS of the peripheral for the
 *   	TDD test.
 *
 * @details
 * 	 	This function enables the LEUART STATUS register to be provided to
 * 	 	a function outside this .c module.
 *
 * @param[in] *leuart
 *   	A pointer to the base address of the leuart peripheral.
 *
 * @return
 * 	 	Returns the STATUS register value as an uint32_t value.
 ******************************************************************************/
uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}



/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/
void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){
	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}





/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/
void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}




/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test.
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   A pointer to the base address of the leuart peripheral.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	//uint8_t temp = data_out;
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}



