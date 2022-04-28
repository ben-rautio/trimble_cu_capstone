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
    , framerate)