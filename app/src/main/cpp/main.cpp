#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>

// Creates a window and fills the screen using software rendering (CPU)
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
            row[x] = 0xFF0000FF; // red, RGBA
        }
    }

    ANativeWindow_unlockAndPost(window);
}