#from pynput.keyboard import Key, Listener

# NOTE: NEED TO RUN THIS WITH SUDO!!!!

import os,sys
import struct
import pdb
import time
from pySerialTransfer.pySerialTransfer import pySerialTransfer as txfer

UPLINK_BAUD = 115200
# PORT = "/dev/ttyTHS1"
PORT = "/dev/ttyUSB0"
#PORT = "COM6"

#Need to add formats for the commands to send the ET

class struct(object):
    z = 10

def main():
    cycle = 0
    try:
        link = txfer.SerialTransfer(PORT, baud=UPLINK_BAUD)
        link.open()
        print('Opened port: {}'.format(PORT))
        testStruct = struct

        while True:
            # Busy wait for msg
            # while True:
            #     if not link.available():
            #         time.sleep(0.1)
            #     else: break

            #testTx(link)

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
                testStruct.z=cycle
                changeBrightness(link, testStruct)
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
    time.sleep(0.5)
    send_size = 0
    str_size = link.tx_obj(cycle.z)
    send_size+=str_size
    link.send(send_size)
    time.sleep(0.5)

def testTx(link):
    time.sleep(0.5)
    send_size = 0
    str_ = "Poopy"
    str_size = link.tx_obj(str_)
    send_size+=str_size
    link.send(send_size)
    time.sleep(0.5)

if __name__ == '__main__':
    main()