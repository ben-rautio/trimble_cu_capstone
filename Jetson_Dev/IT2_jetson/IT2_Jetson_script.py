from pynput.keyboard import Key, Listener
import os,sys
import struct
import pdb
import time
from pySerialTransfer import pySerialTransfer as txfer

UPLINK_BAUD = 115200
# PORT = "/dev/ttyTHS1"
PORT = "COM6"

#Need to add formats for the commands to send the ET

def main():
    cycle = 0
    try:
        link = txfer.SerialTransfer(PORT, baud=UPLINK_BAUD)
        link.open()
        print('Opened port: {}'.format(PORT))

        while True:
            # Busy wait for msg
            # while True:
            #     if not link.available():
            #         time.sleep(0.1)
            #     else: break

            if link.status < 0:
                if link.status == -1:
                    print('ERROR: CRC_ERROR')
                elif link.status == -2:
                    print('ERROR: PAYLOAD_ERROR')
                elif link.status == -3:
                    print('ERROR: STOP_BYTE_ERROR')
            
            user_input = input("Enter a command: ")
            if user_input == " ":
                cycle = (cycle + 10) % 110
                print("Change Duty Cycle: {}".format(cycle))
                changeBrightness(link, cycle)
            elif user_input == "end":
                break
            else: 
                print("Unrecogonized Command: {}".format(user_input))


    except KeyboardInterrupt:
        link.close()
    
    except:
        import traceback
        traceback.print_exc()
        link.close()

def changeBrightness(link, cycle):
    # txBuf = struct.pack(ET_CMD_FORMAT, 4, cycle)
    # for index in range(len(txBuf)):
    #     link.txBuff[index] = txBuf[index]
    # link.send(len(txBuf))
    pass

if __name__ == '__main__':
    main()