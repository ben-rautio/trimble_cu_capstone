#ifndef POWER_COMMANDS_H
#define POWER_COMMANDS_H

//Commands sent to power control
enum class pwr_cmd : uint8_t {CMD_CHANGE_BAT=1,
				CMD_CHANGE_SHORE,
				CMD_HEARTBEAT};

typedef struct {
    bool enabled;
} pwr_change_to_bat;

typedef struct {
    bool enabled;
} pwr_change_to_shore;

typedef struct {
    pwr_cmd cmd;
    union {
        pwr_change_to_bat bat;
        pwr_change_to_shore shore;
    };
} pwr_cmd_t;


//Recieved from power control
//Probably need to put strings in flash on 328pb,
//something like "_f" rather than in static ram. When 
//sending it, copy it from flash to ram and then send it.

enum class pwr_remote_cmd : uint8_t {CMD_PUSHSTATUS= 1,
				 CMD_LOG};

typedef struct {
    uint16_t current;
    uint16_t vbat0; //fixed point *1000
    uint16_t vbat1;
    uint16_t vbat2;
    uint16_t vbat3;
    uint16_t vshore;
    uint16_t active_src; //active power source, shore or battery
} pwr_status_msg_t;

typedef struct
{
  uint8_t message[30];
} pwr_log_msg_t;

typedef struct
{
  pwr_remote_cmd cmd;
  union {
    pwr_status_msg_t status;
    pwr_log_msg_t log;
  };
} pwr_remote_cmd_t;

#endif
