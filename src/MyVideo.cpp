#include "MyVideo.h"

void MyVideo::StartPreview(int device_id, void *hwnd, int width, int height, int fps) {
    try {
        // Set the video capture device format.
        VidDevManager &mgr = Endpoint::instance().vidDevManager();
        MediaFormatVideo format = mgr.getDevInfo(device_id).fmt[0];
        format.width = width;
        format.height = height;
        format.fpsNum = fps;
        format.fpsDenum = 1;
        mgr.setFormat(device_id, format, true);

        // Start the preview on a panel with window handle 'hwnd'.
        // Note that if hwnd is set to NULL, library will automatically create
        // a new floating window for the rendering.
        VideoPreviewOpParam param;
        param.window.handle.window = (void *)hwnd;

        VideoPreview preview(device_id);
        preview.start(param);
    } catch (Error &err) {
    }
}

void MyVideo::listVideoDevices() {
    VidDevManager &vid_mgr = Endpoint::instance().vidDevManager();
    unsigned count = vid_mgr.getDevCount();

    for (unsigned i = 0; i < count; ++i) {
        VideoDevInfo info = vid_mgr.getDevInfo(i);
        std::cout << "Device #" << i << ": "<< info.name << std::endl;
    }
}