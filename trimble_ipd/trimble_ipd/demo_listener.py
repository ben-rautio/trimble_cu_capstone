import math

import rclpy
from rclpy.node import Node

from tf2_ros import LookupException, ConnectivityException, ExtrapolationException, TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from geometry_msgs.msg import PoseStamped
from rclpy.duration import Duration
from transforms3d import quaternions
import numpy as np

class FrameListener(Node):

    def __init__(self):
        super().__init__('camera_tf2_frame_listener')

        self.target_frame = 'Camera'
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.pose_publisher = self.create_publisher(PoseStamped, 'MarkerTreePose', 1000)
        self.timer = self.create_timer(0.05, self.on_timer)

    def on_timer(self):
        from_frame_rel = self.target_frame
        to_frame_rel = 'MarkerTree'

        try:
            now = rclpy.time.Time()
            trans_MarkerTree_Camera = self.tf_buffer.lookup_transform(
                to_frame_rel,
                from_frame_rel,
                now,
                timeout=Duration(seconds=1.0))
        except TransformException as ex:
            self.get_logger().info(
                f'couldnt tf {to_frame_rel} to {from_frame_rel}: {ex}')
            return
        
        try:
            now = rclpy.time.Time()
            trans_Camera_Tractor = self.tf_buffer.lookup_transform(
                'Camera',
                'Tractor',
                now,
                timeout=Duration(seconds=1.0))
        except TransformException as ex:
            self.get_logger().info(
                f'couldnt tf Camera to Tractor: {ex}')
            return

        #must use MarkerTree->Camera, then Camera->Tractor
        #https://matthew-brett.github.io/transforms3d/reference/transforms3d.quaternions.html#transforms3d.quaternions.quat2mat
        msg = PoseStamped()

        #in transforms3d it is [w x y z]
        q_mt_c = [trans_MarkerTree_Camera.transform.rotation.w,
                  trans_MarkerTree_Camera.transform.rotation.x,
                  trans_MarkerTree_Camera.transform.rotation.y,
                  trans_MarkerTree_Camera.transform.rotation.z
                  ]

        #translation of markertree relative to camera from PNP
        tvec_mt_cam = [
            trans_MarkerTree_Camera.transform.translation.x,
            trans_MarkerTree_Camera.transform.translation.y,
            trans_MarkerTree_Camera.transform.translation.z
        ]

        rMatTest = np.matrix(quaternions.quat2mat(q_mt_c))
        trans_fix = np.asarray(-rMatTest.T * (np.matrix(tvec_mt_cam).T), dtype=np.float)

        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = 'Camera'
        
        msg.pose.position.x = trans_fix[0][0]
        msg.pose.position.y = trans_fix[1][0]
        msg.pose.position.z = trans_fix[2][0]
        
        msg.pose.orientation.x = q_mt_c[1]
        msg.pose.orientation.y = q_mt_c[2]
        msg.pose.orientation.z = q_mt_c[3]
        msg.pose.orientation.w = -q_mt_c[0]

        self.pose_publisher.publish(msg)
        # self.get_logger().info("Publishing pose")

def main():
    rclpy.init()
    node = FrameListener()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    rclpy.shutdown()