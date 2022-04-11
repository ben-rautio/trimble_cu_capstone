import numpy as np
import parse_params
import math
import cv2
import pnp
import matrix_util
import traceback
import pySerialTransfer as txfer
#from pySerialTransfer.pySerialTransfer import pySerialTransfer as txfer


#fisheye calibration process
# https://medium.com/@kennethjiang/calibrate-fisheye-lens-using-opencv-333b05afa0b0

#opencv camera calibration
# https://learnopencv.com/camera-calibration-using-opencv/

#projectPoints explanation
# https://answers.opencv.org/question/96474/projectpoints-functionality-question/

#, SCALE_FACTOR, BALANCE,IMG_W,IMG_H,FILENAME,objPts,K,D
########################################################################################
class pipeline:
    def __init__(self):
        self.SCALE_FACTOR = 1.0
        self.BALANCE = 1.0
        self.FILENAME = 'Arducam_Feb04_intrinsic_parameters.json'
        self.objPts = [
                (-499.269, 0.0, 0.0),
                (-258.762,465.137,0.0),
                (238.125,465.137,0.0),
                (496.09375,0.0,0.0)
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
            str_size = self.link.tx_obj(cycle.z)
            send_size+=str_size
            self.link.send(send_size)

    def changeCycle(self, count):
        if count != 4 and self.cycle <= 95:
            cycle = self.cycle + 5
            self.setCycle(cycle)
        elif self.cycle == 100:
            cycle = 10
            self.setCycle(cycle)

        
    def getCalibration(self):
        K,D = parse_params.getCalibrationParams(self.FILENAME)
        K = np.float32(K)
        D = np.float32(D)
        self.K = K
        self.D = D


    def Find_centroids(img):
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
        #print("Moments: " + str(len(ctrds_pnp)))
        
        #display centroid locations
        #for ctrd in ctrds_pnp:
            #label each centroid
            #cv2.circle(img, (int(ctrd[0]), int(ctrd[1])), 1, (0, 255, 0), 8)
            #cv2.putText(img, "ctd " + str(int(ctrd[0])) + ", " + str(int(ctrd[1])), (int(ctrd[0]), int(ctrd[1])),cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 3)

        #pack into array
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

    

    def Find_Pose(self):
        print(pipeline.gstreamer_pipeline(flip_method=2))
        cap = cv2.VideoCapture(pipeline.gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
        window_handle = cv2.namedWindow("Camera", cv2.WINDOW_NORMAL)
        IInd_window = cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
        
        while cap.isOpened():
            ret_val, img = cap.read() 
            if ret_val:
                centroidPts,num_found = self.Find_centroids(img)
                if num_found == 4:
                    #fix the sorting
                    centroidPts.sort()
                    if centroidPts[0][1] < centroidPts[1][1]:
                        centroidPts[0], centroidPts[1] = centroidPts[1], centroidPts[0]
                    if centroidPts[2][1] > centroidPts[3][1]:
                        centroidPts[2], centroidPts[3] = centroidPts[3], centroidPts[2]

                    #undistPts = pnp.undistortPoints(centroidPts, K, D, newK)
                    ret,rvecs,tvecs = cv2.solvePnP(self.objPnts, centroidPts, self.K, self.D, flags = cv2.SOLVEPNP_IPPE)
                    if ret:
                        #retval, rotVec, tVec = pnp.runPNP(np.float32(objPts), np.float32(centroidPts), K, D)
                        rMat, _ = cv2.Rodrigues(rvecs)

                        #Making the rMat into the transform format for the decomposition
                        rMatNew = np.c_[rMat, np.zeros((3,1))]
                        rMatNew = np.r_[rMatNew,np.zeros((1,4))]
                        rMatNew[0:3, 3] = tvecs.reshape(3)
                        rMatNew[3,3] = 1

                        #get euler angles
                        extMat = rMatNew[:3,:4]
                        eulerAngles = cv2.decomposeProjectionMatrix(extMat)[-1]

                        #compute camera pose in world coordinate system (sanity check)
                        cameraWorldPose = -np.matrix(rMat).T * np.matrix(tvecs)

                        #Axis of world system to display in the image
                        cv2.drawFrameAxes(img, self.K, self.D, rvecs, tvecs, 50)

                        #resize to smaller res so its easier to view
                        imS = cv2.resize(img, (1280, 720))

                        keyCode = cv2.waitKey(1) & 0xFF
                        if keyCode == 27:
                            print("Closing...")
                            break
                        cv2.imshow('Camera',imS)
                        if (matrix_util.isRotationMatrix(rMat)):
                            euler_angles = matrix_util.rotationMatrixToEulerAngles(rMat)
                            #print("eulerAngles: \n" + str(eulerAngles))
                            #print("Rvec: " + str(rotVec))
                            #print("Tvec: " + str(tVec))
                            print("cameraWorldPose: \n" + str(cameraWorldPose))

                            y_radians = euler_angles[1]
                            y_degrees = y_radians * (180.0/math.pi)
                            self.YAW = y_degrees
                            #print("Y rotation (RAD): " + str(y_radians))
                            print("Y rotation (DEG): " + str(y_degrees))
                        else: print("INVALID ROTATION MATRIX")
                    else: print("Incorrect number of centroids: " + str(num_found))
                else:
                    self.changeCycle(num_found)



    

if __name__ == '__main__':
    p = pipeline()
    p.getCalibration() 
    p.tryLink()  
    p.Find_Pose()        
                


#end of modular code
##########################################################################################
