#ifndef SI7021_HG
#define SI7021_HG
//***********************************************************************************
// Include files
//***********************************************************************************

//***********************************************************************************
// defined files
//***********************************************************************************

#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"
#include "em_i2c.h"
#include "i2c.h"

//***********************************************************************************
// global variables
//***********************************************************************************
#define 	USING_I2C0
#ifdef		USING_I2C0
	#define		I2C_SCLLOC			I2C_ROUTELOC0_SCLLOC_LOC15
	#define		I2C_SDALOC			I2C_ROUTELOC0_SDALOC_LOC15
#else
	#define		I2C_SCLLOC			I2C_ROUTELOC0_SCLLOC_LOC19
	#define		I2C_SDALOC			I2C_ROUTELOC0_SDALOC_LOC19
#endif

#define		I2C_Enable			true
#define		I2C_Master			true
#define		SDA_Enable			true
#define		SCL_Enable			true
//#define 	I2C_FREQ_FAST_MAX   392157
#define		I2C_RefFreq			0
#define		I2C_Freq			I2C_FREQ_FAST_MAX
#define		ClockHLR			i2cClockHLRAsymetric
#define		command				0xf3
#define 	read				0b1
#define 	write 				0b0
#define 	SA					0x40

//***********************************************************************************
// function prototypes
//***********************************************************************************

void si7021_i2c_open(void);
void si7021_i2c_read(uint32_t read_cb);
float si7021_temp_convert();

#endif /* SRC_HEADER_FILES_SI7021_H_ */
