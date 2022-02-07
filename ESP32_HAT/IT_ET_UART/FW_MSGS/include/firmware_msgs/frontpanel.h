#ifndef FRONTPANEL_H
#define FRONTPANEL_H

/* Messages for communicating with the front panel */


//We send this to the front panel
//These structs are used in the command queue
enum class fp_cmd : uint8_t {CMD_ESTOP = 1,
			       CMD_SETDISPLAY,
			       CMD_RESET_RADIO,
			       CMD_POWER_UPDATE};

typedef struct
{
  bool enabled;
} set_estop_msg_t;

typedef struct
{
  uint8_t row;
  uint8_t msg[DISPLAY_LINE_LEN+1];
} set_display_msg_t;

typedef struct
{
  bool enabled;
} set_radio_msg_t;

typedef struct
{
  uint8_t shore_present;
  uint8_t shore_engaged;
} power_update_msg_t;

typedef struct
{
  fp_cmd cmd;

  union {
    set_estop_msg_t estop;
    set_display_msg_t display;
    set_radio_msg_t radio;
    power_update_msg_t power;
  };
} fp_cmd_t;


//Received from the front panel on a regular basis
enum class fp_remote_cmd : uint8_t {
  CMD_PUSHSTATUS = 1,
    CMD_LOG};

typedef struct
{
  uint8_t radio_state;
  uint8_t sw_state; //since we can be software stopped without the radio holding us back
  uint8_t power_btn;
} fp_status_msg_t;

typedef struct
{
  uint8_t message[30];
} fp_log_msg_t;


typedef struct {
  fp_remote_cmd cmd;
  union
  {
    fp_status_msg_t fp_status;
    fp_log_msg_t log;
    power_update_msg_t pwr;
  };
} fp_remote_cmd_t;

#endif
