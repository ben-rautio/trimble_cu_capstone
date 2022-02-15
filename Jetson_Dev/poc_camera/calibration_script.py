import cv2
import os

# Note: give the script sufficient permissions to write the images to your directory of choice
# EX: sudo chmod calibration_script.py 777
# try adding in openCV's checkerboard detection overlaid on the live feed

#set to true if want to show overlay of detected checkerboard or not
DETECT_CHECKERBOARD = False
CHECKERBOARD_SIZE = (7,9)
CAP_WIDTH = 3264
CAP_HEIGHT = 2464
FRAMERATE = 21
#terminating criteria for iterative algorithms
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

def gstreamer_pipeline(
    capture_width=CAP_WIDTH,
    capture_height=CAP_HEIGHT,
    framerate=FRAMERATE,
    flip_method=2,
):
    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "format=(string)NV12, width=(int)%d, height=(int)%d, "
        "framerate = (fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=GRAY8 ! appsink"
        % (
            capture_width,
            capture_height,
            framerate,
            flip_method,
        )
    )
21
def show_camera():
    pic_num = 0
    #MAKE SURE TO GIVE SUDO PERMISSIONS
    path = './outdoor_calib_2-13-22'
    print(gstreamer_pipeline(flip_method=2))
    cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
    window_handle = cv2.namedWindow("CSI Camera", cv2.WINDOW_NORMAL)
    while cap.isOpened():
        ret_val, img = cap.read()
        if DETECT_CHECKERBOARD:
            print("detect checkerboards")
            retval, corners = cv2.findChessboardCorners(img, CHECKERBOARD_SIZE, flags = cv2.CALIB_CB_FAST_CHECK)
            #retval, corners = cv2.findChessboardCorners(img, CHECKERBOARD_SIZE, None)
            if retval:
                # points3d = []
                # points2d = []
                #print("subpix corners")
                # corners2 = cv2.cornerSubPix(img, corners, (11,11), (-1,-1), criteria)

                #draw & display corners
                print("draw checkerboard")
                cv2.drawChessboardCorners(img, CHECKERBOARD_SIZE, corners, retval)
            else:
                print("NO CHECKERBOARD DETECTED")
        if ret_val:
            keyCode = cv2.waitKey(30) & 0xFF
            if keyCode == 27:
                print("Closing...")
                break
            elif keyCode == 32:
                print("saving image: " + os.path.join(path , 'imx219_calib_'+str(pic_num)+'.jpg'))
                if not cv2.imwrite(os.path.join(path , 'imx219_calib_'+str(pic_num)+'.jpg'), img):
                    print("failed to save img")
                pic_num+=1
            cv2.imshow("CSI Camera", img)
    cv2.destroyAllWindows()
    cap.release()


if __name__ == "__main__":
    show_camera()