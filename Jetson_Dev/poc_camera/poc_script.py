#Will calculate centroids with 3264 x 2464 resolution. Not a good idea to display it though...

import cv2
import numpy as np
import matplotlib.pyplot as plt
# import PnP_Solver


#Try these two pipelines: 
# nvarguscamerasrc ! video/x-raw(memory:NVMM), format=NV12, width=1920, height=1080, framerate=30/1 ! nvvidconv ! video/x-raw, format=NV12 ! appsink
#nvarguscamerasrc ! video/x-raw(memory:NVMM), format=NV12, width=1920, height=1080, framerate=30/1 ! nvvidconv ! video/x-raw, format=GRAY8 ! appsink
# Need nvvidconc to copy from nvmm to cpu memory, since appsink cannot read from nvmm
#refer to this: https://forums.developer.nvidia.com/t/feeding-nv12-into-opencv2/167626

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

def show_camera():
    # To flip the image, modify the flip_method parameter (0 and 2 are the most common)
    print(gstreamer_pipeline(flip_method=2))
    cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=2), cv2.CAP_GSTREAMER)
    window_handle = cv2.namedWindow("CSI Camera", cv2.WINDOW_AUTOSIZE)
    while cap.isOpened():
        ret_val, img = cap.read()
        #slice out the UV channels
        #img = img[:2464, :]
        #SHOULD WE HAVE BLUR OR NO? DO BEFORE OR AFTER THRESHOLDING?
        blur = cv2.GaussianBlur(img,(5,5),0)
        if ret_val:
            #CONSIDER DOING THIS IN THE PIPELINE OR DELETE SINCE JUST HAVING INTENSITY WITH NV12
            #Do we even need to grayscale if using nv12 image?
            #img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            # print("SHAPE: " + str(img.shape))
            # print("DTYPE: " + str(img.dtype))
            # print("MAX: " + str(img.max()))
            ret, thresh = cv2.threshold(blur, 200, 255, cv2.THRESH_BINARY)
            #currently uses 7x7 erosion kernel with 2 iterations
            thresh = cv2.erode(thresh, np.ones((7,7), np.uint8), iterations=3)
            contours, hierarchy = cv2.findContours(image=thresh, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
            print("Contours: " + str(len(contours)))
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

    #         cv2.drawContours(image=thresh, contours=contours, contourIdx=-1, color=(255, 0, 0), thickness=10, lineType=cv2.LINE_AA)
    #         keyCode = cv2.waitKey(30) & 0xFF
    #         if keyCode == 27:
    #             cv2.imwrite('sd_test_44deg.jpg', thresh)
    #             break
    # cv2.destroyAllWindows()
    cap.release()


if __name__ == "__main__":
    show_camera()