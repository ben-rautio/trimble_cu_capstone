import pnp
import numpy as np
import parse_params
import math
import cv2
import IT2_Jetson_centroids as pipeline
import matrix_util

def draw(img, corners, imgpts):
    corner = tuple(corners[0].ravel())
    img = cv2.line(img, corner, tuple(imgpts[0].ravel()), (255,0,0), 5)
    img = cv2.line(img, corner, tuple(imgpts[1].ravel()), (0,255,0), 5)
    img = cv2.line(img, corner, tuple(imgpts[2].ravel()), (0,0,255), 5)
    return img

FISHEYE_MODEL = False
#FISHEYE_MODEL = True

SCALE_FACTOR = 1.0
BALANCE = 1.0
IMG_W = 3264
IMG_H = 2464
FILENAME = 'Arducam_Feb04_intrinsic_parameters.json'

#############################################################################################
#x,y,z in mm
objPts = np.array([
    (0.0, 0.0, 0.0),
    (0.0,0.0,62.0),
    (0.0,170.0,62.0),
    (0.0,170.0,0.0)
    ])
print("objPts: " + str(objPts))

K,D = parse_params.getCalibrationParams(FILENAME)

if FISHEYE_MODEL:
    newK = pnp.getInKNewForFisheye(SCALE_FACTOR,IMG_W,IMG_H,BALANCE,K,D)
else: 
    newK = K

#############################################################################################
#find centroids
# img = cv2.imread("bread_brd_tilt_pnp.jpg",cv2.IMREAD_GRAYSCALE)
# window_handle = cv2.namedWindow("CSI Camera", cv2.WINDOW_NORMAL)
print(pipeline.gstreamer_pipeline(flip_method=2))
cap = cv2.VideoCapture(pipeline.gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
window_handle = cv2.namedWindow("Camera", cv2.WINDOW_NORMAL)
while cap.isOpened():
    ret_val, img = cap.read()
    if ret_val:
        ret, thresh = cv2.threshold(img, 200, 255, cv2.THRESH_BINARY)
        #currently uses 7x7 erosion kernel with 2 iterations
        thresh = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=2)
        contours, hierarchy = cv2.findContours(image=thresh, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
        #print("Contours: " + str(contours))
        #print("Count: " + str(len(contours)))
        # if len(contours) != 4:
        #     print("Not enough contours, count: " + str(len(contours)))
        # else:
        ctrds_pnp = []
        for c in contours:
            #calculate moments for each contour
            M = cv2.moments(c)
            if M["m00"] != 0:
                cX = M["m10"] / M["m00"]
                cY = M["m01"] / M["m00"]
                #print("Centroid: X %f, Y %f" % (cX, cY))
                ctrds_pnp.append((cX,cY))
                #print("Y: " + str(img[cY,cX]))
            else:
                cX, cY = 0, 0
        print("Moments: " + str(len(ctrds_pnp)))
        # keyCode = cv2.waitKey(1) & 0xFF
        # img1 = thresh.copy()
        # contours1 = contours[0].reshape(-1,2)

        # for (x, y) in contours1:
        #     cv2.circle(img1, (x, y), 1, (255, 0, 0), 3)

        #cv2.drawContours(thresh, contours, -1, (0,255,0), 3)
        #cv2.imshow("CSI Camera", img1)
        # while 1:
        #     keyCode = cv2.waitKey(1) & 0xFF
        #     if keyCode == 27:
        #         print("Closing...")
        #         break
        #pack into array
        ctrd = []
        for ctroid in ctrds_pnp:
            ctrd.append([ctroid[0],ctroid[1]])
        centroidPts = ctrd
        #print("centroidPts: " + str(centroidPts))
        #############################################################################################

        #############################################################################################
        #then, do the following after finding centroids:
        # print("undistPts: " + str(undistPts))
        if (len(ctrds_pnp) == 4):
            undistPts = pnp.undistortPoints(centroidPts, K, D, newK)
            retval, rotVec, tVec = pnp.runPNP(objPts, undistPts)
            # print("retval: " + str(retval))
            # print("rotVec: " + str(rotVec))
            # print("tVec: " + str(tVec))
            #print("reproj_error: " + str(reproj_error))
            rMat, _ = cv2.Rodrigues(rotVec)
            #print("RMat: " + str(rMat))

            ###############################################################################################
            axis = np.float32([[0,0,0], [0,1,0], [0,0,-1]]).reshape(-1,3)
             # project 3D points to image plane
            imgpts, jac = cv2.projectPoints(axis, rotVec, tVec, np.float32(newK), np.float32(np.zeros((4,1))))
            print("K: " + str(K))
            print(ctrds_pnp)
            print(imgpts)
            ###############################################################################################

            img_axis = draw(img,ctrds_pnp,imgpts)
            keyCode = cv2.waitKey(1) & 0xFF
            if keyCode == 27:
                print("Closing...")
                break
            cv2.imshow('Camera',img_axis)
            if (matrix_util.isRotationMatrix(rMat)):
                euler_angles = matrix_util.rotationMatrixToEulerAngles(rMat)
                print("Euler Angles: " + str(euler_angles))
                print("Rvec: " + str(rotVec))
                print("Tvec: " + str(tVec))
                y_radians = euler_angles[1]
                print("Y rotation (RAD): " + str(y_radians))
                print("Y rotation (DEG): " + str(y_radians * (180.0/math.pi)))
            else:
                print("INVALID ROTATION MATRIX")
        else: print("Incorrect number of moments: " + str(len(ctrds_pnp)))