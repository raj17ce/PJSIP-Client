#include <iostream>
#include <pjsua2.hpp>

using namespace pj;

class MyVideo{
   public:
    static void StartPreview(int device_id, void *hwnd, int width, int height, int fps);
    static void listVideoDevices();
};