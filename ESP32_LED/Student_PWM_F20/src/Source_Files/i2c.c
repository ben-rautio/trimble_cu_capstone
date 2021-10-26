/**
 * @file 	i2c.c
 * @author 	Alexander Bork
 * @date 	9/29/2020
 * @brief 	Contains i2c driver functions.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
//** Standard Libraries
//** Silicon Lab include files
//** User/developer include files
#include "i2c.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t TXB_STATUS_MASK = 0b10000000;
static uint32_t START_CMD = 0b1;
static uint32_t STOP_CMD = 0b10;
static uint32_t I2C_FLAG_MSTOP = 0b100000000;
static uint32_t I2C_IFC_CLEAR = 0x7ffcf;
static uint32_t I2C_IEN_CLEAR = 0x0;

static I2C_STATE_MACHINE	i2c_SM;

//***********************************************************************************
// Private functions prototypes
//***********************************************************************************

static void i2c_bus_reset();
static void i2c_ackIRPT();
static void i2c_nackIRPT();
static void i2c_mstopIRPT();
static void i2c_rxdatavIRPT();

//***********************************************************************************
// Private functions
//***********************************************************************************


/***************************************************************************//**
 * @brief
 *		Resets the I2C bus.
 *
 * @details
 *		Saves the state of the IEN register, then disables all interrupts. Clears any
 *		pending IFs and then clears the I2C transmit buffer. Uses a simultaneous setting
 *		of the start and stop bits to STOP the buffer and will stall until the MSTOP
 *		interrupt flag has been set. Then it resets the state of the IEN register and
 *		resets the state of the peripheral I2C state machine by writing an abort bit.
 *
 * @note
 *		Does not write anything from the master to the slave, or read anything
 *		from the slave to the master.
 *
 * @param[in] *i2c
 *		A pointer to the base address of one of the two i2c peripherals.
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c){
	if (i2c->STATE & I2C_STATE_BUSY){
		i2c->CMD = I2C_CMD_ABORT;
		while (i2c->STATE & I2C_STATE_BUSY);	// wait for the Abort function to clear the busy bit
	}
	uint32_t saveIEN = i2c->IEN;		// Save which interrupts are enabled
	i2c->IEN = I2C_IEN_CLEAR;			// Disable all interrupts
	i2c->IFC = I2C_IFC_CLEAR;			// Clear IF register
	i2c->CMD = TXB_STATUS_MASK;			// Clear the Transmit Buffer
	if(i2c->STATUS & TXB_STATUS_MASK){
		i2c->CMD = START_CMD | STOP_CMD;		// Simultaneously set the start and stop bits
	}
	while(!(i2c->IF & I2C_FLAG_MSTOP));	// Stall until MSTOP is set
	i2c->IFC = i2c->IF;					// Clear IF register
	i2c->IEN = saveIEN;					// Re-enable interrupts
	i2c->CMD = I2C_CMD_ABORT;			//
}

//***********************************************************************************
// Global functions
//***********************************************************************************


/***************************************************************************//**
 * @brief
 *		Initializes the I2C peripheral for functionality.
 *
 * @details
 *		Takes in pointers to i2c and i2c_setup to initialize the I2C peripheral.
 *		Enables the clock to the I2C peripheral and passes the values from i2c_setup
 *		to the I2C peripheral. Routes both the SDA and SCL lines to the SI7021 and enables
 *		the routing from the I2C.
 *
 * @note
 * 		Enables ACK, NACK, RXDATAV, and MSTOP interrupts.
 * @param[in] *i2c
 *		A pointer to the base address of one of the two i2c peripherals.
 *
 * @param[in] i2c_setup
 * 		A pointer to a struct that is used to initialize the i2c peripheral.
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup){
	if(i2c == I2C0){
		CMU_ClockEnable(cmuClock_I2C0, true);
	}
	if(i2c == I2C1){
		CMU_ClockEnable(cmuClock_I2C1, true);
	}

	if((i2c->IF & 0x01) == 0){
		i2c->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	}
	else{
		i2c->IFC = 0x01;
		EFM_ASSERT(!(i2c->IF & 0x01));
	}


	I2C_Init_TypeDef i2c_values;

	i2c_values.enable = i2c_setup->enable;
	i2c_values.master = i2c_setup->master;
	i2c_values.refFreq = i2c_setup->refFreq;
	i2c_values.freq = i2c_setup->freq;
	i2c_values.clhr = i2c_setup->clhr;

	I2C_Init(i2c, &i2c_values);

	i2c->ROUTEPEN = i2c_setup->SDA_EN | (i2c_setup->SCL_EN << 1);
	i2c->ROUTELOC0 = i2c_setup->SDALOC | i2c_setup->SCLLOC;

	i2c_bus_reset(i2c);

	i2c->IFC = i2c->IF;				// Clear IF register
	i2c->IEN &= ~i2c->IEN;		// Clear IEN register
	i2c->IEN |= i2c_irpts;		// Enable ACK, NACK, RXDATAV, MSTOP interrupts
	if(i2c == I2C0){
		NVIC_EnableIRQ(I2C0_IRQn);
	}
	else{
		NVIC_EnableIRQ(I2C1_IRQn);
	}
}



/**************************s*************************************************//**
 * @brief
 *		Initializes the I2C State Machine.
 *
 * @details
 *		After initializing the State Machine, send the Start bit from the I2C
 *		to the slave and send the slave address OR'd with the write bit to
 *		initialize the slave for writing the command.
 *
 * @note
 *		Does not enable or disable any interrupts.
 *
 * @param[in] *i2c
 *		A pointer to the base address of one of the two i2c peripherals.
 *
 * @param[in] SA
 *		The address of the slave that the I2C will be using.
 *
 * @param[in] command
 *		The command that the I2C will send to the slave for reading.
 *
 * @param[in] read
 *		The variable that defines what a read bit is for sending commands.
 *
 * @param[in] write
 *		The variable that defines what a write bit is for sending commands.
 *
 * @param[in] *data
 *		A pointer to the address of the variable that will store the temperature
 *		from the si7021.
 *
 * @param[in] read_cb
 * 		The value of the si7021_read_cb bit == 0b1000.
 *
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, uint32_t SA, uint32_t command, uint32_t read, uint32_t write, uint32_t *data, uint32_t read_cb){

	EFM_ASSERT((i2c->STATE & _I2C_STATE_MASK) == I2C_STATE_STATE_IDLE);

	sleep_block_mode(i2c_Sleep_Block);	// Keep the PG out of EM2

	i2c_SM.SM_Busy = true;			// Bool for if the state machine is busy
	i2c_SM.states = START_COMM;		// Initialize the state for starting communication
	i2c_SM.si7021_address = SA;		// Set the device address
	i2c_SM.command = command;		// Set the read command in the SM
	i2c_SM.read = read;				// Set read bit
	i2c_SM.write = write;			// Set write bit
	i2c_SM.device1_cb = read_cb;	// Sets the callback value 0b1000
	i2c_SM.data = data;				// Pointer to an address to store the data
	i2c_SM.I2Cn = i2c;				// Puts the address of the I2C peri in the state machine

	i2c_SM.I2Cn->CMD = I2C_START;	// Writes the state to the CMD register
	i2c_SM.I2Cn->TXDATA = (i2c_SM.si7021_address << 1)| i2c_SM.write;	// Send a write bit

}



/***************************************************************************//**
 * @brief
 *		Function that handles any I2C ack interrupts.
 *
 * @details
 *		The switch statement in the function will send commands or switch
 *		the state of the I2C State Machine depending on which state the ACK
 *		interrupt occurs in.
 *
 *
 * @note
 *		If an ACK occurs in any state after receiving the Most significant byte
 *		then an EFM_ASSERT(false) will be triggered signifying that a false condition
 *		has occurred.
 *
 ******************************************************************************/
void i2c_ackIRPT(void){
	switch(i2c_SM.states){
		case START_COMM:
			i2c_SM.I2Cn->CMD |= I2C_CMD_CLEARTX;
			i2c_SM.I2Cn->TXDATA = i2c_SM.command;
			i2c_SM.states = SEND_CMD;
			break;
		case SEND_CMD:
			i2c_SM.I2Cn->CMD = I2C_CMD_CLEARTX;
			i2c_SM.I2Cn->CMD = I2C_START;
			i2c_SM.I2Cn->TXDATA = (i2c_SM.si7021_address << 1) | i2c_SM.read;
			i2c_SM.states = WAIT_FOR_RESPONSE;
			break;
		case WAIT_FOR_RESPONSE:
			i2c_SM.I2Cn->CMD |= I2C_CMD_CLEARTX;
			i2c_SM.states = RECEIVE_MS;
			break;
		case RECEIVE_MS:		// Receive this ack when I2C acks the MS byte
			break;
		case RECEIVE_LS:		// False condition
			EFM_ASSERT(false);
			break;
		case END_TRANS:			// False condition
			EFM_ASSERT(false);
			break;
		default:				// Default handling
			EFM_ASSERT(false);
			break;
	}
}


/***************************************************************************//**
 * @brief
 *		Function that handles any I2C NACK interrupts.
 *
 * @details
 *		The Nack function will only do anything in the Wait for response state
 *		if the slave NACKs the start and read commands. In the wait for response
 *		state, it will resend the start bit and send the slave address OR'd with
 *		the read bit through the transmit buffer.
 *
 * @note
 *		If a NACK occurs in any state besides during Wait for response, then
 *		an EFM_ASSERT(false) will be triggered signifying that a false condition
 *		has occurred.
 *
 ******************************************************************************/
void i2c_nackIRPT(void){
	switch(i2c_SM.states){
		case START_COMM:		// False condition
			EFM_ASSERT(false);
			break;
		case SEND_CMD:			// False condition
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_RESPONSE:	// A nack in WFR state will stay in WFR
			i2c_SM.I2Cn->CMD = I2C_START;
			i2c_SM.I2Cn->TXDATA = (i2c_SM.si7021_address << 1) | i2c_SM.read;
			break;
		case RECEIVE_MS:		// False condition
			EFM_ASSERT(false);
			break;
		case RECEIVE_LS:		// Receive this nack when I2C finishes reading the LS byte
			EFM_ASSERT(false);
			break;
		case END_TRANS:			// False condition
			EFM_ASSERT(false);
			break;
		default:				// Default handling
			EFM_ASSERT(false);
			break;
	}
}


/***************************************************************************//**
 * @brief
 *		Function that handles any I2C MSTOP interrupts.
 *
 * @details
 *		The only case where an MSTOP interrupt will occur is in the End
 *		transmission state, where the switch case will unblock EM2 reset the state
 *		machine to the first state and add the SI7021 read callback to the scheduler.
 *
 *
 * @note
 *		If an MSTOP occurs in any state besides END transmission then an
 *		EFM_ASSERT(false) will be triggered signifying that a false condition
 *		has occurred.
 *
 ******************************************************************************/
void i2c_mstopIRPT(void){
	switch(i2c_SM.states){
		case START_COMM:		// False condition
			EFM_ASSERT(false);
			break;
		case SEND_CMD:			// False condition
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_RESPONSE:	// False condition
			EFM_ASSERT(false);
			break;
		case RECEIVE_MS:		// False condition
			EFM_ASSERT(false);
			break;
		case RECEIVE_LS:		// False condition
			EFM_ASSERT(false);
			break;
		case END_TRANS:
			sleep_unblock_mode(i2c_Sleep_Block);
			i2c_SM.states = START_COMM;
			add_scheduled_event(i2c_SM.device1_cb);
			i2c_SM.SM_Busy = false;
			break;
		default:				// Default Handling
			EFM_ASSERT(false);
			break;
	}
}



/***************************************************************************//**
 * @brief
 *		Function that handles any I2C RXDATAV interrupts.
 *
 * @details
 *		The switch statement in the function will send commands or switch
 *		the state of the I2C State Machine depending on which state the RXDATAV
 *		interrupt occurs in. An RXDATAV signifies that there is data ready in the
 *		RXDATA buffer and is ready to be read.
 *
 *
 * @note
 *		If an RXDATAV occurs in any state besides receiving the Most significant and
 *		least significant bytes, then an EFM_ASSERT(false) will be triggered signifying
 *		that a false condition has occurred.
 *
 ******************************************************************************/
void i2c_rxdatavIRPT(void){
	switch(i2c_SM.states){
		case START_COMM:		// False condition
			EFM_ASSERT(false);
			break;
		case SEND_CMD:			// False condition
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_RESPONSE:	// False condition
			EFM_ASSERT(false);
			break;
		case RECEIVE_MS:
			*i2c_SM.data = i2c_SM.I2Cn->RXDATA << 8;
			i2c_SM.I2Cn->CMD = I2C_MASTER_ACK;
			i2c_SM.states = RECEIVE_LS;
			break;
		case RECEIVE_LS:
			*i2c_SM.data |= i2c_SM.I2Cn->RXDATA;
			i2c_SM.I2Cn->CMD = I2C_CMD_NACK;
			i2c_SM.I2Cn->CMD = I2C_CMD_STOP;
			i2c_SM.states = END_TRANS;
			break;
		case END_TRANS:			// False condition
			EFM_ASSERT(false);
			break;
		default:				// Default handling
			EFM_ASSERT(false);
			break;
	}
}


/***************************************************************************//**
 * @brief
 *		Interrupt Handler for I2C0
 *
 * @details
 *		A variable int_flag will store any interrupt flag if it has also been enabled.
 *		All of the interrupts will then be cleared from I2C0->IF. The
 *		function handles the scheduling of processing each interrupt.
 *
 * @note
 *		Int_flag will be used to service every interrupt, with the priority give to
 *		ACK, then RXDATAV, then NACK, then MSTOP.
 *
 ******************************************************************************/
void I2C0_IRQHandler(){								// If using I2C0
	uint32_t int_flag;
	int_flag = I2C0->IF & I2C0->IEN;
	I2C0->IFC = int_flag;
	if(int_flag & I2C_IF_ACK){
		i2c_ackIRPT();
	}
	if(int_flag & I2C_IF_RXDATAV){
		i2c_rxdatavIRPT();
	}
	if(int_flag & I2C_IF_NACK){
		i2c_nackIRPT();
	}
	if(int_flag & I2C_IF_MSTOP){
		EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));
		i2c_mstopIRPT();
	}
}


/***************************************************************************//**
 * @brief
 *		Interrupt Handler for I2C1.
 *
 * @details
 *		A variable int_flag will store any interrupt flag if it has also been enabled.
 *		All of the interrupts will then be cleared from I2C1->IF. The
 *		function handles the scheduling of processing each interrupt.
 *
 * @note
 *		Int_flag will be used to service every interrupt, with the priority give to
 *		ACK, then RXDATAV, then NACK, then MSTOP.
 *
 ******************************************************************************/
void I2C1_IRQHandler(){								// If using I2C1
	uint32_t int_flag;
	int_flag = I2C1->IF & I2C1->IEN;
	I2C1->IFC = int_flag;
	if(int_flag & I2C_IF_ACK){
		i2c_ackIRPT();
	}
	if(int_flag & I2C_IF_RXDATAV){
		i2c_rxdatavIRPT();
	}
	if(int_flag & I2C_IF_NACK){
		i2c_nackIRPT();
	}
	if(int_flag & I2C_IF_MSTOP){
		EFM_ASSERT(!(I2C0->IF & I2C_IF_MSTOP));
		i2c_mstopIRPT();
	}
}



