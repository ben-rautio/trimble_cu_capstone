#!/usr/bin/python3
from pySerialTransfer import pySerialTransfer as txfer
import os,sys
import struct
import pdb
import time

UPLINK_BAUD = 9600
STATUS_FORMAT ='<BB'
ESTOP_CMD_FORMAT = '<BB'
SHORE_CMD_FORMAT = '<BB'
DISPLAY_CMD_FORMAT='<BB20s'

def main():
    if len(sys.argv) == 2:
        port = sys.argv[1]
    else:
        port = '/dev/ttyUSB0'
        
    try:
        link = txfer.SerialTransfer(port, baud=UPLINK_BAUD)
        link.open()
        print('Opened port: {}'.format(port))
        msgCount = 0
        while True:
            ###################################################################
            # Wait for a response and report any errors while receiving packets
            ###################################################################

        
            #This busy waits...
            while True:
                if not link.available():
                    time.sleep(0.1)
                else:
                    break

            
            if link.status < 0:
                if link.status == -1:
                    print('ERROR: CRC_ERROR')
                elif link.status == -2:
                    print('ERROR: PAYLOAD_ERROR')
                elif link.status == -3:
                    print('ERROR: STOP_BYTE_ERROR')
                    
            if link.bytesRead != struct.calcsize(STATUS_FORMAT):
                print('Message is incorrect size')
                continue

            buff = bytes(link.rxBuff[0:link.bytesRead])
            (radio_stat, sw_stat) = struct.unpack(STATUS_FORMAT, buff)

            ###################################################################
            # Display the received data
            ###################################################################
            #print('SENT: {} {} {}'.format(list_, str_, float_))
            print('%d RCVD: radio:%d sw:%d' % (msgCount, radio_stat, sw_stat))
            msgCount += 1
            if msgCount % 5 == 0:
                #Set the line 3 on the display:
                updateDisplay(link, 3, msgCount)

            #Alter the estop state:
            if msgCount % 25 == 0:
                '''
                if radio_stat == 1:
                    updateRadioReset(link)
                updateEStop(link, 1 if msgCount % 50 == 0 else 0)
                '''
                updateShoreStatus(link, (True, False)
                                  
           
    except KeyboardInterrupt:
        link.close()
    
    except:
        import traceback
        traceback.print_exc()
        link.close()

def updateShoreStatus(link, state):
    #Magic numbers come out of BaseState.h
    txBuf = struct.pack(SHORE_CMD_FORMAT, 4, state)
    for index in range(len(txBuf)):
        link.txBuff[index] = txBuf[index]
    link.send(len(txBuf))
    
def updateRadioReset(link):
    #Magic numbers come out of BaseState.h
    txBuf = struct.pack(ESTOP_CMD_FORMAT, 3, True)
    for index in range(len(txBuf)):
        link.txBuff[index] = txBuf[index]
    link.send(len(txBuf))
    
def updateEStop(link, state):
    txBuf = struct.pack(ESTOP_CMD_FORMAT, 1, state)
    for index in range(len(txBuf)):
        link.txBuff[index] = txBuf[index]
    link.send(len(txBuf))
    
def updateDisplay(link, line, count):

    txBuf = struct.pack(DISPLAY_CMD_FORMAT, 2, line, b'Msg: %d' % count)
    for index in range(len(txBuf)):
        link.txBuff[index] = txBuf[index]
    link.send(len(txBuf))

    
if __name__ == '__main__':
    main()
