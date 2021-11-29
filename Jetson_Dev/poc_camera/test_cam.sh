gst-launch-1.0 nvarguscamerasrc ! 'video/x-raw(memory:NVMM), width=3264, height=2464, format=NV12, framerate=21/1' ! nvvidconv flip-method=2 ! nvoverlaysink

