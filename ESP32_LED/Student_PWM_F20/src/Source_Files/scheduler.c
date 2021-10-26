/**
 * @file 	scheduler.c
 * @author 	Alexander Bork
 * @date 	9/24/2020
 * @brief 	Contains scheduler functions for interrupt processing
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "scheduler.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;

//***********************************************************************************
// Private functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *		Initializes the event_scheduled static variable.
 *
 * @details
 *		Sets event_scheduled to 0, meaning no scheduled events.
 *
 * @note
 *		CLEAR_EVENTS is defined as 0.
 *
 ******************************************************************************/
void scheduler_open(void){
	event_scheduled = CLEAR_EVENTS;
}


/***************************************************************************//**
 * @brief
 *		Adds a particular event to be scheduled.
 *
 * @details
 *		Bitwise ORs the parameter event into the event_scheduled variable
 *		so that it can be processed.
 *
 * @note
 *		Uses CORE statements to have the process atomic; i.e. un-interruptible.
 *
 * @param[in] event
 * 		A binary value to be added to the event_scheduled variable.
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled |= event;
	CORE_EXIT_CRITICAL();
}



/***************************************************************************//**
 * @brief
 *		Removes a particular event from the schedule.
 *
 * @details
 *		Bitwise ANDs the negation of event with the event_scheduled variable
 *		to remove that particular event from the schedule.
 *
 * @note
 *		Uses CORE statements to have the process atomic; i.e. un-interruptible.
 *
 * @param[in] event
 * 		A binary value to be removed from the event_scheduled variable.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event){
	CORE_DECLARE_IRQ_STATE;
	CORE_ENTER_CRITICAL();
	event_scheduled &= ~event;
	CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
 * @brief
 *		Returns all events scheduled.
 *
 * @details
 *		Returns the event_scheduled variable, where each bit represents a
 *		particular scheduled event.
 *
 * @note
 *		Does not have any parameters.
 *
 * @return
 *		The event_scheduled variable.
 ******************************************************************************/
uint32_t get_scheduled_events(void){
	return event_scheduled;
}
