# https://stackoverflow.com/questions/59062410/how-can-i-solvepnp-with-fisheye-camera-parameters
# https://stackoverflow.com/questions/61147903/what-is-the-correct-way-to-undistort-points-captured-using-fisheye-camera-in-ope

#Must undistort the image points first

import cv2
import numpy as np

#do i need to worry about translating deepen Params to 
# acceptable openCV2 params?
def undistortPoints(points, inK, inD, inKNew):
    #convert intrinsic matrix to array
    K = np.asarray(inK)
    #same thing to the distortion coeff
    d = np.asarray(inD)
    #same thing for distorted points
    distPoints = np.asarray(points)
    distPoints = distPoints[:, 0:2].astype('float32')
    #Make empty array for undistorted
    undistPoints = np.empty_like(distPoints)
    #insert new axis
    distPoints = np.expand_dims(distPoints, axis=1)
    res = np.squeeze(cv2.fisheye.undistortPoints(point))
