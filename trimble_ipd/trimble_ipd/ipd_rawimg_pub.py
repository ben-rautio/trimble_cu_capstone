from numpy import rate
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
from trimble_ipd.pnp_modular import pipeline
# from pipeline import gstreamer_pipeline
import numpy as np
import math


#for publishing tfs from PNP
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster
import transforms3d
from os.path import exists
import os

#intr cal file
CAL_FILE = 'cam_cal.yaml'
SAVE_PATH = '/home'
USER = ''
CAL_PATH = os.path.join(SAVE_PATH, USER,CAL_FILE)

def gstreamer_pipeline(
    capture_width=3264,
    capture_height=2464,
    framerate=21,
    flip_method=2,
):

    return (
        # self.pipeline.tryLink()
        # self.pipeline.getCalibration()
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
    , framerate)

class CsiPublisher(Node):

    def __init__(self):
        camcfg, rate = gstreamer_pipeline()
        super().__init__('csi_raw_publisher')
        self.publisher = self.create_publisher(Image, 'csi_raw', 30)
        #self.timer_period = 1.0/rate
        #self.timer = self.create_timer(self.timer_period, self.timer_callback)
        # Add error checking here for if the camera cannot open the stream!
        self.cap = cv2.VideoCapture((camcfg), cv2.CAP_GSTREAMER)
        self.bridge = CvBridge()

        self.calibrated = False
        self.pipeline = pipeline()
        self.pipeline.tryLink()
        self.br = TransformBroadcaster(self)

    def pub_img(self):
        ret, frame = self.cap.read()
        #print("img pub function")
        if ret == True:
            self.publisher.publish(self.bridge.cv2_to_imgmsg(frame, encoding='passthrough'), )

            if (self.calibrated == False):
                if (exists(CAL_PATH)):
                    self.pipeline.getCalibration()
                    self.calibrated = True
                else:
                    self.get_logger().info("Intrinsic calibration doesn't exist, not publishing MarkerTree->Cam TF until calibrated")
            else:
                ret_pnp, rvec, tvec, euler_angles, rMat = self.pipeline.Find_Pose(frame)
                if ret_pnp:
                    self.broadcast_tf(euler_angles, rMat, tvec)
        else:
            self.get_logger().info('unable to read frame')
    
    def broadcast_tf(self, euler_angles, rMat, tvec):
        # str_pitch = "pitch: " + str(euler_angles[0] * (180.0 / math.pi))
        str_yaw = "yaw: " + str(euler_angles[1] * (180.0 / math.pi))    
        # str_roll = "roll: " + str(euler_angles[2] * (180.0 / math.pi))
        self.get_logger().info("tf MarkerTree->Camera found")

        # self.get_logger().info(str_pitch)
        self.get_logger().info(str_yaw)
        # self.get_logger().info(str_roll)
        
        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'MarkerTree'
        t.child_frame_id = 'Camera'

        t.transform.translation.x = tvec[0][0]
        t.transform.translation.y = tvec[1][0]
        t.transform.translation.z = tvec[2][0]

        q = transforms3d.quaternions.mat2quat(rMat)
        t.transform.rotation.x = q[0]
        t.transform.rotation.y = q[1]
        t.transform.rotation.z = q[2]
        t.transform.rotation.w = q[3]

        self.br.sendTransform(t)

def main(args=None):
    rclpy.init()

    csi_raw_pub = CsiPublisher()

    while(rclpy.ok()):
        #print("while loop")
        csi_raw_pub.pub_img()
        # if (rclpy.ok()):
        #     print("if statement")
        #     rclpy.spin_once(csi_raw_pub)

    csi_raw_pub.cap.release()
    csi_raw_pub.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()


