#Will calculate centroids with 3264 x 2464 resolution. Not a good idea to display it though...

import cv2
import numpy as np
import matplotlib.pyplot as plt
# import PnP_Solver

import IT2_Jetson_UART as uart
import os,sys
import struct
import pdb
import time
#from pySerialTransfer.pySerialTransfer import pySerialTransfer as txfer

#UPLINK_BAUD = 115200
# PORT = "/dev/ttyTHS1"
#PORT = "/dev/ttyUSB0"
#PORT = "COM6"

#Need to add formats for the commands to send the ET

def gstreamer_pipeline(
    capture_width=3264,
    capture_height=2464,
    framerate=21,
    flip_method=2,
):

    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "format=NV12, width=(int)%d, height=(int)%d, "
        "framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, format=GRAY8 ! appsink"
        % (
            capture_width,
            capture_height,
            framerate,
            flip_method,
        )
    )


def detectCentroids(c):
    print(gstreamer_pipeline(flip_method=2))
    cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
    window_handle = cv2.namedWindow("CSI Camera", cv2.WINDOW_AUTOSIZE)
    while cap.isOpened():
        ret_val, img = cap.read()
        #slice out the UV channels
        #img = img[:2464, :]
        #SHOULD WE HAVE BLUR OR NO? DO BEFORE OR AFTER THRESHOLDING?
        #blur = cv2.GaussianBlur(img,(5,5),0)
        if ret_val:
            #CONSIDER DOING THIS IN THE PIPELINE OR DELETE SINCE JUST HAVING INTENSITY WITH NV12
            #Do we even need to grayscale if using nv12 image?
            #img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            # print("SHAPE: " + str(img.shape))
            # print("DTYPE: " + str(img.dtype))
            # print("MAX: " + str(img.max()))
            ret, thresh = cv2.threshold(img, 200, 255, cv2.THRESH_BINARY)
            #currently uses 7x7 erosion kernel with 2 iterations
            thresh = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=3)
            contours, hierarchy = cv2.findContours(image=thresh, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
            print("Contours: " + str(len(contours)))
            if len(contours) != 4:
                changeBrightness(c)
            else:
                for c in contours:
                    #calculate moments for each contour
                    M = cv2.moments(c)
                    if M["m00"] != 0:
                        cX = int(M["m10"] / M["m00"])
                        cY = int(M["m01"] / M["m00"])
                        print("Centroid: X %d, Y %d" % (cX, cY))
                        #print("Y: " + str(img[cY,cX]))
                    else:
                        cX, cY = 0, 0

    cap.release()


def changeBrightness(c):

    user_input = input("Enter a command: ")

    if user_input == "10":
        print("Change Duty Cycle: {}".format("10"))
        c.setCycle(10)
    elif user_input == "0":
        print("Change Duty Cycle: {}".format("0"))
        c.setCycle(0)
    elif user_input == "20":
        print("Change Duty Cycle: {}".format("20"))
        c.setCycle(20)
    elif user_input == "30":
        print("Change Duty Cycle: {}".format("30"))
        c.setCycle(30)
    elif user_input == "40":
        print("Change Duty Cycle: {}".format("40"))
        c.setCycle(40)
    elif user_input == "50":
        print("Change Duty Cycle: {}".format("50"))
        c.setCycle(50)
    elif user_input == "60":
        print("Change Duty Cycle: {}".format("60"))
        c.setCycle(60)
    elif user_input == "70":
        print("Change Duty Cycle: {}".format("70"))
        c.setCycle(70)
    elif user_input == "80":
        print("Change Duty Cycle: {}".format("80"))
        c.setCycle(80)
    elif user_input == "90":
        print("Change Duty Cycle: {}".format("90"))
        c.setCycle(90)
    elif user_input == "100":
        print("Change Duty Cycle: {}".format("100"))
        c.setCycle(100)
    elif user_input == "110":
        print("Change Duty Cycle: {}".format("110"))
        c.setCycle(110)
    elif user_input == "-10":
        print("Change Duty Cycle: {}".format("-10"))
        c.setCycle(-10)
    elif user_input == "end":
        c.setCycle(10)
    else: 
        print("Unrecogonized Command: {}".format(user_input))


def main():
    c = uart.talker()  
    detectCentroids(c)  
    
    

if __name__ == '__main__':
    main()