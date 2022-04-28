
import numpy as np
#import parse_params
import math
import cv2
#import pnp
#import matrix_util
import traceback
# import pySerialTransfer as txfer
import json
import yaml
import numpy as np
import os
from trimble_ipd.pySerialTransfer import pySerialTransfer as txfer
import sys
sys.path.append('/pySerialTransfer/pySerialTransfer')

criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 20, 0.1)


def isRotationMatrix(R) :
    Rt = np.transpose(R)
    shouldBeIdentity = np.dot(Rt, R)
    I = np.identity(3, dtype = R.dtype)
    n = np.linalg.norm(I - shouldBeIdentity)
    return n < 1e-6

def rotationMatrixToEulerAngles(R) :

    assert(isRotationMatrix(R))

    sy = math.sqrt(R[0,0] * R[0,0] +  R[1,0] * R[1,0])

    singular = sy < 1e-6

    if  not singular :
        x = math.atan2(R[2,1] , R[2,2])
        y = math.atan2(-R[2,0], sy)
        z = math.atan2(R[1,0], R[0,0])
    else :
        x = math.atan2(-R[1,2], R[1,1])
        y = math.atan2(-R[2,0], sy)
        z = 0

    return np.array([x, y, z])



#fisheye calibration process
# https://medium.com/@kennethjiang/calibrate-fisheye-lens-using-opencv-333b05afa0b0

#opencv camera calibration
# https://learnopencv.com/camera-calibration-using-opencv/

#projectPoints explanation
# https://answers.opencv.org/question/96474/projectpoints-functionality-question/

#, SCALE_FACTOR, BALANCE,IMG_W,IMG_H,FILENAME,objPts,K,D
########################################################################################
def getCalibrationParams(filename):
    f = open(filename)

    data = yaml.safe_load(f)

    cameraMatrix = np.zeros((3, 3),dtype=np.longdouble)
    cameraMatrix[0][0] = data['fx']
    cameraMatrix[1][1] = data['fy']
    cameraMatrix[0][2] = data['cx']
    cameraMatrix[1][2] = data['cy']
    cameraMatrix[2][2] = 1
    #print(cameraMatrix)

    distCoeff = np.zeros((1,5), dtype = np.longdouble)
    distCoeff[0][0] = data['k1']
    distCoeff[0][1] = data['k2']
    distCoeff[0][2] = data['p1']
    distCoeff[0][3] = data['p2']
    distCoeff[0][4] = data['k3']

    return (cameraMatrix,distCoeff)

class pipeline:
    def __init__(self):
        self.SCALE_FACTOR = 1.0
        self.BALANCE = 1.0
        self.FILENAME = '/home/cam_cal.yaml'
        self.objPts = [
                (-497.0, 0.0, 0.0),
                (-250.0, -341.0, 0.0),
                (249.0, -346.0, 0.0),
                (500.0, 0.0, 0.0)
        ]
        self.K = None
        self.D = None
        self.YAW = None

        #UART
        self.cycle = 0
        self.port = "/dev/ttyTHS1"
        self.baud = 115200
        self.link = txfer.SerialTransfer(self.port, self.baud)

    def tryLink(self):
        try:
            self.link.open()
            print('Opened port: {}'.format(self.port))

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
            str_size = self.link.tx_obj(cycle)
            send_size+=str_size
            self.link.send(send_size)

    def changeCycle(self, count):
        if count != 4 and self.cycle <= 90:
            #cycle = self.cycle + 10
            cycle = 10
            self.cycle = 10
            print("changeCycle: " + str(self.cycle))
            self.setCycle(cycle)
        elif self.cycle == 100:
            #cycle = 10
            self.setCycle(cycle)

        
    def getCalibration(self):
        K,D = getCalibrationParams(self.FILENAME)
        K = np.float32(K)
        D = np.float32(D)
        self.K = K
        self.D = D


    def Find_centroids(self,img):
        ret, thresh = cv2.threshold(img, 200, 255, cv2.THRESH_BINARY)
        #currently uses 7x7 erosion kernel with 2 iterations
        eroded = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=2)
        contours, hierarchy = cv2.findContours(image=eroded, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)

        ctrds_pnp = []
        for c in contours:
            #calculate moments for each contour
            M = cv2.moments(c)
            if M["m00"] != 0:
                cX = M["m10"] / M["m00"]
                cY = M["m01"] / M["m00"]
                ctrds_pnp.append((cX,cY))
            else:
                cX, cY = 0, 0
        
        ctrd = []
        for ctroid in ctrds_pnp:
            ctrd.append([ctroid[0],ctroid[1]])
        centroidPts = ctrd
        num_found = len(ctrds_pnp)
        
        return centroidPts, num_found

    
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

    

    def Find_Pose(self, img):
        centroidPts,num_found = self.Find_centroids(img)
        if num_found == 4:
            #fix the sorting
            centroidPts.sort()
            if centroidPts[0][1] < centroidPts[1][1]:
                centroidPts[0], centroidPts[1] = centroidPts[1], centroidPts[0]
            if centroidPts[2][1] > centroidPts[3][1]:
                centroidPts[2], centroidPts[3] = centroidPts[3], centroidPts[2]

            ret,rvecs,tvecs = cv2.solvePnP(np.float32(self.objPts), np.float32(centroidPts), np.float32(self.K), np.float32(self.D), flags = cv2.SOLVEPNP_IPPE)

            if ret:
                rvec_refine, tvec_refine = cv2.solvePnPRefineLM(np.float32(self.objPts), np.float32(centroidPts), np.float32(self.K), np.float32(self.D), rvecs, tvecs, criteria)
                rMat, _ = cv2.Rodrigues(rvec_refine)
                #[P Y R] or [X Y Z] in degrees
                rMatTp = rMat.transpose()
                euler_angles = [math.atan2(-rMatTp[2][1], rMatTp[2][2]),
                                math.asin(rMatTp[2][0]),
                                math.atan2(-rMatTp[1][0], rMatTp[0][0])] 
                return ret, rvec_refine, tvec_refine, euler_angles, rMat
            else: 
                print("PNP failed")
        else:
            ret = False
            self.changeCycle(num_found)
            print("Incorrect number of centroids: " + str(num_found))
        return ret ,None, None, None, None

if __name__ == '__main__':
    p = pipeline()
    p.getCalibration() 
    p.tryLink()  
    p.Find_Pose()        
                


#end of modular code
##########################################################################################