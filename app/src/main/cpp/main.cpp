#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>

void fill_window(ANativeWindow* window) {
    if (!window) return;

    uint16_t winW = ANativeWindow_getWidth(window);
    uint16_t winH = ANativeWindow_getHeight(window);

    ANativeWindow_setBuffersGeometry(window, winW, winH, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(window, &buffer, nullptr) < 0) {
        return;
    }

    for (int y = 0; y < winH; y++) {
        uint32_t* row = (uint32_t*)((char*) buffer.bits + y * buffer.stride * 4);
        for (int x = 0; x < winW; x++) {
            row[x] = 0xFF0000FF; // purple, RGBA
        }
    }

    ANativeWindow_unlockAndPost(window);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    // Hook the window creation callback
    activity->callbacks->onNativeWindowCreated = [](ANativeActivity* act, ANativeWindow* window) {
        fill_window(window);
    };
}