/**
 * @file 	SI7021.c
 * @author 	Alexander Bork
 * @date 	9/29/2020
 * @brief 	Contains SI7021 temperature sensor functions.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
//** Standard Libraries
//** Silicon Lab include files
//** User/developer include files
#include "SI7021.h"

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t temp_result;

//***********************************************************************************
// Private functions prototypes
//***********************************************************************************



//***********************************************************************************
// Private functions
//***********************************************************************************




//***********************************************************************************
// Global functions
//***********************************************************************************


/***************************************************************************//**
 * @brief
 *		Initializes the I2C peripheral.
 *
 * @details
 *		Declares an open struct and initializes all of the data members of that
 *		struct. Then it passes the struct and the I2C peripheral to i2c_open(...)
 *		so that the I2C peripheral can be initialized using the struct.
 *
 * @note
 *		All of the Macros for initializing the struct are defined in SI7021.h.
 *
 ******************************************************************************/
void si7021_i2c_open(void){
	I2C_OPEN_STRUCT i2c_7021_setup;

	i2c_7021_setup.refFreq = I2C_RefFreq;
	i2c_7021_setup.freq = I2C_Freq;
	i2c_7021_setup.clhr = ClockHLR;
	i2c_7021_setup.enable =	I2C_Enable;
	i2c_7021_setup.master =	I2C_Master;
	i2c_7021_setup.SDALOC = I2C_SDALOC;
	i2c_7021_setup.SDA_EN = SDA_Enable;
	i2c_7021_setup.SCLLOC =	I2C_SCLLOC;
	i2c_7021_setup.SCL_EN =	SCL_Enable;

	i2c_open(I2C0, &i2c_7021_setup);
}


/***************************************************************************//**
 * @brief
 *		Calls the i2c_start function.
 *
 * @details
 *		Passes all of the variables that initialize the I2C state machine into the
 *		i2c_start function.
 *
 * @note
 *		Is only used for encapsulation purposes so that app.c does not talk
 *		directly to i2c.c.
 *
 * @param[in] read_cb
 * 		The macro that defines the read callback for the scheduler.
 *
 *
 ******************************************************************************/
void si7021_i2c_read(uint32_t read_cb){
	i2c_start(I2C0, SA, command, read, write, &temp_result, read_cb);
}



/***************************************************************************//**
 * @brief
 *		Convert the temperature to Fahrenheit.
 *
 * @details
 *		Reads the variable 'temp_result', and convert the variable into float
 *		fahrenheit so that it can be compared to 80 in determining whether to
 *		turn on the LED1 or not.
 *
 * @note
 *		The algorithm for convert the temperature to fahrenheit was given by the
 *		SI7021 datasheet.
 *
 * @return
 *		The temperature converted to Fahrenheit.
 ******************************************************************************/
float si7021_temp_convert(){
	float cTemp = ((175.72 * temp_result)/65536) - 46.85;
	float fTemp = (1.8 * cTemp) + 32;
	return fTemp;
}










