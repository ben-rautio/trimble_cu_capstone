# firmware_msgs

This repo contains the header files used to define structures passed between firmware components via their serial links. These are NOT ROS messages. 

This repo is required because standard ROS messages are overlaid in the include namespace when run through rosserial's message generation. ROS messages to talk to the firmware are all located in estop_msgs.
