import json
import os
import numpy as np
def getCalibrationParams(filename):
    f = open(filename)

    data = json.load(f)

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


#test codez
#print(getCalibrationParams('Arducam_Feb04_intrinsic_parameters.json'))