import tf2_ros
import tf_transformations
from geometry_msgs import TransformStamped  
from ipd_inferface.srv import SetExtCal

import rclpy
from rclpy.node import Node
import yaml
# import transforms3d


class EXT_Service(Node):
    def __init__(self):
        # print('hi')
        super().__init__('ext_service')
        self.srv = self.create_service(SetExtCal, 'get_extcamera_info', self.send_response_message)
        
    def send_response_message(self, request, response):
        extrinsics = request.transform # Get the extrinsic params
        if(request.transform):
            ext_cal = {"x": extrinsics.Vector3.x, "y": extrinsics.Vector3.y, "z": extrinsics.Vector3.z, "qx": extrinsics.Quaternion.x, "qy": extrinsics.Quaternion.y, "qz": extrinsics.Quaternion.z, "qw": extrinsics.Quaternion.w}
            
            home = Path('/home')
            yamlpath = home / 'ext_cal.yaml'
            yamlpath.write_text(yaml.dump(cam_info_dict))

            response.success = True
            # response.status_message = "Valid calibration: Position = " + str(extrinsics.Vector3) + ", Quaternion = " + str(extrinsics.Quarternion) + "."
        else:
            response.success = False
            # response.status_message = "Calibration failed"
        return response
        
def main(args=None):
    rclpy.init(args=args)
    
    extService = EXT_Service()
    
    rclpy.spin(extService)
    
    rclpy.shutdown()
    
if __name__ == '__main__':
    main()
    
