#ifndef SRC_HEADER_FILES_I2C_HG
#define SRC_HEADER_FILES_I2C_HG

//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"
//***********************************************************************************
// defined files
//***********************************************************************************
#include "em_i2c.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"

//***********************************************************************************
// global variables
//***********************************************************************************

#define i2c_irpts			0x1E0
#define i2c_Sleep_Block		2
//#define I2C_IF_RXDATAV		0b100000UL		// Bit 6 of IF reg
//#define I2C_IF_ACK			0b1000000UL		// Bit 7 of IF reg
//#define I2C_IF_NACK			0b10000000UL	// Bit 8 of IF reg
//#define I2C_IF_MSTOP		0b100000000UL	// Bit 9 of IF reg
#define I2C_START			0b1UL
#define I2C_STOP			0b10UL
#define I2C_MASTER_ACK		0b100UL
#define I2C_MASTER_NACK		0b1000UL
#define I2C_BUSHELD			0x80

typedef struct {
	bool	 				enable;
	bool	 				master;
	uint32_t				refFreq;
	uint32_t				freq;
	I2C_ClockHLR_TypeDef	clhr;
	uint32_t				SDALOC;
	bool					SDA_EN;
	uint32_t				SCLLOC;
	bool					SCL_EN;
} I2C_OPEN_STRUCT;

typedef enum{
	START_COMM,
	SEND_CMD,
	WAIT_FOR_RESPONSE,
	RECEIVE_MS,
	RECEIVE_LS,
	END_TRANS
} DEFINED_STATES;

typedef struct {
	DEFINED_STATES 	states;			// An enum to keep track of the state
	uint32_t		si7021_address;	// 7 bit address of device to be read
	uint32_t		command;		// An integer used to send the read command
	uint32_t		*data;			// A pointer to the
	uint32_t		read;			// Read is 0b1
	uint32_t		write;			// Write is 0b0
	uint32_t		device1_cb;
	bool			SM_Busy;		// True is State machine is busy
	I2C_TypeDef	*	I2Cn;			// Keeps track of whether it is I2C0 or 1
} I2C_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_TypeDef *i2c, uint32_t SA, uint32_t command, uint32_t read, uint32_t write, uint32_t *data, uint32_t read_cb);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

#endif /* SRC_HEADER_FILES_I2C_H_ */
