import pnp
import numpy as np
import parse_params
import math
import cv2
import IT2_Jetson_centroids as pipeline
import matrix_util

#fisheye calibration process
# https://medium.com/@kennethjiang/calibrate-fisheye-lens-using-opencv-333b05afa0b0

#opencv camera calibration
# https://learnopencv.com/camera-calibration-using-opencv/

#projectPoints explanation
# https://answers.opencv.org/question/96474/projectpoints-functionality-question/

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
objPts = [
    (0.0, 0.0, 0.0),
    (0.0,62.0,0.0),
    (170.0,62.0,0.0),
    (170.0,0.0,0.0)
    ]
#print("objPts: " + str(objPts))

K,D = parse_params.getCalibrationParams(FILENAME)

if FISHEYE_MODEL:
    newK = pnp.getInKNewForFisheye(SCALE_FACTOR,IMG_W,IMG_H,BALANCE,K,D)
else: 
    newK = K

K = np.float32(K)
D = np.float32(D)

#############################################################################################
def live_PNP():
    #find centroids
    print(pipeline.gstreamer_pipeline(flip_method=2))
    cap = cv2.VideoCapture(pipeline.gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
    window_handle = cv2.namedWindow("Camera", cv2.WINDOW_NORMAL)
    IInd_window = cv2.namedWindow("Output", cv2.WINDOW_NORMAL)
    while cap.isOpened():
        ret_val, img = cap.read()
        if ret_val:
            ret, thresh = cv2.threshold(img, 200, 255, cv2.THRESH_BINARY)
            #currently uses 7x7 erosion kernel with 2 iterations
            thresh = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=2)
            contours, hierarchy = cv2.findContours(image=thresh, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)

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
            print("Moments: " + str(len(ctrds_pnp)))
            
            for ctrd in ctrds_pnp:
                #label each centroid
                cv2.circle(img, (int(ctrd[0]), int(ctrd[1])), 1, (0, 255, 0), 8)
                cv2.putText(img, "ctd " + str(int(ctrd[0])) + ", " + str(int(ctrd[1])), (int(ctrd[0]), int(ctrd[1])),cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 3)

            #pack into array
            ctrd = []
            for ctroid in ctrds_pnp:
                ctrd.append([ctroid[0],ctroid[1]])
            centroidPts = ctrd

            imS2 = cv2.resize(img, (1280, 720))

            keyCode = cv2.waitKey(1) & 0xFF
            if keyCode == 27:
                print("Closing...")
                break
            cv2.imshow('Output',imS2)
            #############################################################################################

            #############################################################################################
            #then, do the following after finding centroids:
            if (len(ctrds_pnp) == 4):

                #fix the sorting
                centroidPts.sort()
                if centroidPts[0][1] < centroidPts[1][1]:
                    centroidPts[0], centroidPts[1] = centroidPts[1], centroidPts[0]
                if centroidPts[2][1] > centroidPts[3][1]:
                    centroidPts[2], centroidPts[3] = centroidPts[3], centroidPts[2]

                #undistPts = pnp.undistortPoints(centroidPts, K, D, newK)
                retval, rotVec, tVec = pnp.runPNP(np.float32(objPts), np.float32(centroidPts), K, D)
                rMat, _ = cv2.Rodrigues(rotVec)

                #Making the rMat into the transform format for the decomposition
                rMatNew = np.c_[rMat, np.zeros((3,1))]
                rMatNew = np.r_[rMatNew,np.zeros((1,4))]
                rMatNew[0:3, 3] = tVec.reshape(3)
                rMatNew[3,3] = 1

                #get euler angles
                extMat = rMatNew[:3,:4]
                eulerAngles = cv2.decomposeProjectionMatrix(extMat)[-1]

                #compute camera pose in world coordinate system (sanity check)
                cameraWorldPose = -np.matrix(rMat).T * np.matrix(tVec)

                #Axis of world system to display in the image
                cv2.drawFrameAxes(img, K, D, rotVec, tVec, 50)

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
                    #print("Y rotation (RAD): " + str(y_radians))
                    print("Y rotation (DEG): " + str(y_radians * (180.0/math.pi)))
                else:
                    print("INVALID ROTATION MATRIX")
            else: print("Incorrect number of moments: " + str(len(ctrds_pnp)))

def single_image_test():
    #############################################################################################
    #find centroids
    IMG_USED = "bread_brd_pnp.jpg"
    print("Image Used for PNP: " + IMG_USED + "******************************************************")
    img = cv2.imread(IMG_USED,cv2.IMREAD_GRAYSCALE)
    window_handle = cv2.namedWindow("CSI Camera", cv2.WINDOW_NORMAL)
    ret, thresh = cv2.threshold(img, 200, 255, cv2.THRESH_BINARY)
    #currently uses 7x7 erosion kernel with 2 iterations
    thresh = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=2)
    contours, hierarchy = cv2.findContours(image=thresh, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)

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
    print("Moments: " + str(len(ctrds_pnp)))
    # keyCode = cv2.waitKey(1) & 0xFF

    for ctrd in ctrds_pnp:
        cv2.circle(img, (int(ctrd[0]), int(ctrd[1])), 1, (0, 255, 0), 8)
        cv2.putText(img, "ctd " + str(int(ctrd[0])) + ", " + str(int(ctrd[1])), (int(ctrd[0]), int(ctrd[1])),cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 3)
    # cv2.drawContours(img, contours, -1, (0,255,0), 10)
    imS = cv2.resize(img, (1280, 720))
    cv2.imshow("CSI Camera", imS)
    while 1:
        keyCode = cv2.waitKey(1) & 0xFF
        if keyCode == 27:
            print("Closing...")
            break

    #pack into array
    ctrd = []
    for ctroid in ctrds_pnp:
        ctrd.append((ctroid[0],ctroid[1]))
    centroidPts = ctrd

    print("centroidPts: \n" + str(centroidPts))
    print("objPts: \n" + str(objPts))
    #############################################################################################

    #############################################################################################
    #then, do the following after finding centroids:
    # print("undistPts: " + str(undistPts))
    if (len(ctrds_pnp) == 4):
        #undistPts = pnp.undistortPoints(centroidPts, K, D, newK)

        #This is pretty frustrating now. How do We know how to order the points live?
        #need to do something more
        #fix the sorting
        centroidPts.sort()
        if centroidPts[0][1] < centroidPts[1][1]:
            centroidPts[0], centroidPts[1] = centroidPts[1], centroidPts[0]
        if centroidPts[2][1] > centroidPts[3][1]:
            centroidPts[2], centroidPts[3] = centroidPts[3], centroidPts[2]

        #print("reordered pts: \n" + str(centroidPts_sorted))
        retval, rotVec, tVec = pnp.runPNP(np.float32(objPts), np.float32(centroidPts), K, D)
        print("retval: " + str(retval))
        print("rotVec: " + str(rotVec))
        print("tVec: " + str(tVec))
        rMat, _ = cv2.Rodrigues(rotVec)
        print("RMat: " + str(rMat))
        rMatNew = np.c_[rMat, np.zeros((3,1))]
        rMatNew = np.r_[rMatNew,np.zeros((1,4))]
        print(rMatNew[0:3, 3])
        rMatNew[0:3, 3] = tVec.reshape(3)
        rMatNew[3,3] = 1
        # print("rMatNew: \n" + str(rMatNew))
        #get the location of the origin of the World coordinate system in the camera frame
        worldPntOrig = np.zeros((4,1))
        worldPntOrig[3] = 1
        # print("worldPntOrig: \n" + str(worldPntOrig))
        #Do the matrix mult to get it
        transformResult = np.matmul(rMatNew, worldPntOrig)
        # print("transformResult: \n" + str(transformResult))

        #get euler angles
        extMat = rMatNew[:3,:4]
        # print("extMat: \n" + str(extMat))
        eulerAngles = cv2.decomposeProjectionMatrix(extMat)[-1]
        print("eulerAngles: \n" + str(eulerAngles))

        #get camera pose in world coordinates
        cameraWorldPose = -np.matrix(rMat).T * np.matrix(tVec)
        print("cameraWorldPose: \n" + str(cameraWorldPose))

        ###############################################################################################
        test_points = np.float32([0,0,0])
        # project 3D points to image plane
        imgpts, jac = cv2.projectPoints(test_points, rotVec, tVec, K, D)
        # imgpts, jac = cv2.projectPoints(test_points, rotVec, tVec, np.eye(3), np.zeros((4,1)))
        print("Camera Matrix: \n " + str(K))
        print("Dist Coeff: \n" + str(D))
        # print("test_points: " + str(test_points))
        # print("imgpts: " + str(imgpts))
        # print("K: " + str(K))
        #print(ctrds_pnp)
        # print(imgpts)
        cv2.drawFrameAxes(img, K, D, rotVec, tVec, 50)
        #cv2.circle(img, (int(imgpts[0][0][0]), int(imgpts[0][0][1])), 1, (255, 255, 255), 30)
        #cv2.putText(img, "origin", (int(imgpts[0][0][0]), int(imgpts[0][0][1])),cv2.FONT_HERSHEY_SIMPLEX, 10, (255, 255, 255), 10)
        imS = cv2.resize(img, (1280, 720))
        cv2.imwrite('frameAxis.jpg', img)
        cv2.imshow('CSI Camera',imS)
        while 1:
            keyCode = cv2.waitKey(1) & 0xFF
            if keyCode == 27:
                print("Closing...")
                break
        #img_axis = draw(img,ctrds_pnp,imgpts)
        ###############################################################################################

        #img_axis = draw(img,ctrds_pnp,imgpts)
        # keyCode = cv2.waitKey(1) & 0xFF
        # if keyCode == 27:
        #     print("Closing...")
        # cv2.imshow('Camera',img)
        if (matrix_util.isRotationMatrix(rMat)):
            euler_angles = matrix_util.rotationMatrixToEulerAngles(rMat)
            # print("Euler Angles: " + str(euler_angles))
            # print("Rvec: " + str(rotVec))
            # print("Tvec: " + str(tVec))
            y_radians = euler_angles[1]
            print("Y rotation (RAD): " + str(y_radians))
            print("Y rotation (DEG): " + str(y_radians * (180.0/math.pi)))
        else:
            print("INVALID ROTATION MATRIX")
    else: print("Incorrect number of moments: " + str(len(ctrds_pnp)))

if __name__ == '__main__':
    # single_image_test()
    live_PNP()