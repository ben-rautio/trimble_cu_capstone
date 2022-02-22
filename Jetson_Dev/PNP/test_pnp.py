import pnp
import numpy as np
import parse_params
import math
import cv2
import matrix_util

FISHEYE_MODEL = False
#FISHEYE_MODEL = True

SCALE_FACTOR = 1.0
BALANCE = 1.0
IMG_W = 3264
IMG_H = 2464
FILENAME = 'Arducam_Feb04_intrinsic_parameters.json'

objPts = np.asarray([[0.0, -500.0, 0.0],[0.0,500.0,0.0],[0.0,250.0,500.0],[0.0,-250.0,500.0]])

K,D = parse_params.getCalibrationParams(FILENAME)

if FISHEYE_MODEL:
    newK = pnp.getInKNewForFisheye(SCALE_FACTOR,IMG_W,IMG_H,BALANCE,K,D)
else: 
    newK = K

#then, do the following after finding centroids:

# undistPts = pnp.undistortPoints(centroidPts, K, D, newK)
# rotVec, tVec = pnp.runPNP(objPts, undistPts)
# rMat = cv2.Rodrigues(rotVec)
# if (matrix_util.isRotationMatrix(rMat)):
#     print(matrix_util.rotationMatrixToEulerAngles(rMat))
# else:
#     print("INVALID ROTATION MATRIX")