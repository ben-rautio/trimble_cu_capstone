# https://stackoverflow.com/questions/59062410/how-can-i-solvepnp-with-fisheye-camera-parameters
# https://stackoverflow.com/questions/61147903/what-is-the-correct-way-to-undistort-points-captured-using-fisheye-camera-in-ope

#Must undistort the image points first
# undistortRectify https://docs.opencv.org/3.4/db/d58/group__calib3d__fisheye.html#ga384940fdf04c03e362e94b6eb9b673c9
# PNP examples https://docs.opencv.org/4.x/d7/d53/tutorial_py_pose.html
# PNP docs: https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#ga50620f0e26e02caa2e9adc07b5fbf24e

import cv2
import numpy as np

#we actually don't need this, SolvePNP undistorts points for us...
#do i need to worry about translating deepen Params to 
# acceptable openCV2 params?
# points: 
# inK: intrinsics Matrix
# inD: distoriton coeffs
# inKNew: new cam matrix for undistortion/rectification from 
# estimateNewCameraMatrixforUndistortRectify
# for example: The undistorted image can be seen as taken by a 
# camera with the new K from this function WITHOUT distortion
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
    #undistort them, squeeze gets rid of axis of length one
    res = np.squeeze(cv2.fisheye.undistortPoints(distPoints, K, d))

    #conver to np array
    kNew = np.asarray(inKNew)
    fx = kNew[0,1]
    fy = kNew[1,1]
    cx = kNew[0,2]
    cy = kNew[1,2]

    #convert the undistorted points back to pixel coordinates
    # with inKNew
    for i, (px,py) in enumerate(res):
        undistPoints[i,0] = px * fx + cx
        undistPoints[i,1] = py * fy + cy

    return undistPoints

# Note: Try standard PNP, but then try IPPE. It looks like it 
# is more accurate and faster than other methods with low n 
# and marker trees
# Also try RANSAC approach
# Termination criteria may be useful to guarantee accuracy: https://stackoverflow.com/questions/18955760/how-does-cvtermcriteria-work-in-opencv
# running solvePNP with fisheye: https://stackoverflow.com/questions/59062410/how-can-i-solvepnp-with-fisheye-camera-parameters
# maybe just for testing now, use defaults of solvePNP. this is the iterative methos
# I THINK the units for objPnts are mm, but double check

#imgPnts: UNDISTORTED image points using fisheye calibration
#objPnts: Marker tree coordinates
def runPNP(objPnts, imgPnts):
    ret,rvecs,tvecs = cv2.solvePNP(objPnts, imgPnts, np.eye(3), np.zeros(1,5))
    if ret:
        return (rvecs, tvecs)
    else:
        print("error in pose estimation")
        return (-1,-1)