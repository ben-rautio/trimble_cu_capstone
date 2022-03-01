# NOTE: NEED TO RUN THIS WITH SUDO!!!!

import os,sys
import struct
import pdb
import time
import traceback
from pySerialTransfer.pySerialTransfer import pySerialTransfer as txfer

##class struct(object):
#   z = 10

class talker:
    def __init__(self, port = "/dev/ttyTHS1", baud = 115200):
        self.cycle = 0
        self.port = port
        self.baud = baud
        self.link = txfer.SerialTransfer(port, baud)
        try:
            self.link.open()
            print('Opened port: {}'.format(port))

            #check for errors
            if self.link.status < 0:
                if self.link.status == -1:
                    print('ERROR: CRC_ERROR')
                elif self.link.status == -2:
                    print('ERROR: PAYLOAD_ERROR')
                elif self.link.status == -3:
                    print('ERROR: STOP_BYTE_ERROR')
        except:
            traceback.print_exc()
            self.link.close()


    def setCycle(self, cycle):
        if self.cycle <= 100 and self.cycle >= 0:
            self.cycle = cycle 
            #send the message 
            send_size = 0
            str_size = self.link.tx_obj(cycle.z)
            send_size+=str_size
            self.link.send(send_size)
        else:
            print("choose a duty cycle 0-100")

    def testTx(self):
        #time.sleep(0.5)
        send_size = 0
        str_ = "This is a test"
        str_size = self.link.tx_obj(str_)
        send_size+=str_size
        self.link.send(send_size)
        #time.sleep(0.5)
    
    
    def close_link(self):
        self.link.close()






    # def changeBrightness(link, cycle):
    #     #time.sleep(0.5)
    #     send_size = 0
    #     str_size = link.tx_obj(cycle.z)
    #     send_size+=str_size
    #     link.send(send_size)
    #     #time.sleep(0.5)



            
        # user_input = input("Enter a command: ")
        # if user_input == " ":
        #     cycle = (cycle + 10) % 110
        #     print("Change Duty Cycle: {}".format(cycle))
        #     testStruct.z=cycle
        #     changeBrightness(link, testStruct)
        # elif user_input == "end":
        #     break
        # else: 
        #     print("Unrecogonized Command: {}".format(user_input))




