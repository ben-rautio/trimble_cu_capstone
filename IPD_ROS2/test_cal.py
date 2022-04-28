from sensor_msgs.srv import SetCameraInfo

import rclpy
from rclpy.node import Node
import yaml
from pathlib import Path

class CalService(Node):

    def __init__(self):
        super().__init__('cal_service')

        self.srv = self.create_service(SetCameraInfo, 'set_camera_info', self.set_camera_info_callback)

    def set_camera_info_callback(self, request, response):
        cam_info = request.camera_info
        k = cam_info.k
        d = cam_info.d

        cam_info_dict = {"fx": k[0], "fy": k[3], "cx": k[2], "cy": k[5], "k1": d[0], "k2": d[1], "k3": d[4], "p1": d[2], "p2": d[3], 'camera_model': cam_info.distortion_model}

        home = Path('/home')
        yamlpath = home / 'cam_cal.yaml'
        yamlpath.write_text(yaml.dump(cam_info_dict))

        flag = True

        response.success = flag
        if flag:
            response.status_message = "Success: size = (" + str(cam_info.height) + "," + str(cam_info.width) + "), model = " + cam_info.distortion_model
        else:
            response.status_message = "Calibration failed."

        self.get_logger().info(response.status_message)
        return response

def main(args=None):
    print("Hi from test_cal.py")
    rclpy.init(args=args)

    cal_service = CalService()

    rclpy.spin(cal_service)

    rclpy.shutdown()

if __name__ == '__main__':
    main()
