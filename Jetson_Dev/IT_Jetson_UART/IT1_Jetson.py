#from pynput.keyboard import Key, Listener

# NOTE: NEED TO RUN THIS WITH SUDO!!!!

import os,sys
import struct
import pdb
import time
from pySerialTransfer.pySerialTransfer import pySerialTransfer as txfer

UPLINK_BAUD = 115200
PORT = "/dev/ttyTHS1"
# PORT = "/dev/ttyUSB0"
# PORT = "COM10"

#Need to add formats for the commands to send the ET

class struct(object):
    z = 10

#hello 
def main():
    cycle = 0
    try:
        link = txfer.SerialTransfer(PORT, baud=UPLINK_BAUD)
        link.open()
        print('Opened port: {}'.format(PORT))
        testStruct = struct

        while True:

            if link.status < 0:
                if link.status == -1:
                    print('ERROR: CRC_ERROR')
                elif link.status == -2:
                    print('ERROR: PAYLOAD_ERROR')
                elif link.status == -3:
                    print('ERROR: STOP_BYTE_ERROR')
            
            user_input = input("Enter a command: ")
            if user_input == "10":
                cycle = 10
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "0":
                cycle = 0
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "20":
                cycle = 20
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "30":
                cycle = 30
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "40":
                cycle = 40
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "50":
                cycle = 50
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "60":
                cycle = 60
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "70":
                cycle = 70
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "80":
                cycle = 80
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "90":
                cycle = 90
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "100":
                cycle = 100
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "110":
                cycle = 110
                print("Change Duty Cycle: {}".format(cycle))
                testStruct.z=cycle
                changeBrightness(link, testStruct)
            elif user_input == "-10":
                cycle = -10
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