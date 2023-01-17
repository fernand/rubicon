#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

struct ScreenshotInfo
{
    Display *display;
    Window root;
    int width;
    int height;
};

ScreenshotInfo get_screenshot_info()
{
    Display *display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);
    return ScreenshotInfo{.display = display, .root = root, .width = gwa.width, .height = gwa.height};
}

void screenshot(ScreenshotInfo info, uint8_t *buffer)
{
    XImage *image = XGetImage(info.display, info.root, 0, 0, info.width, info.height, AllPlanes, ZPixmap);
    unsigned long red_mask = image->red_mask;
    unsigned long green_mask = image->green_mask;
    unsigned long blue_mask = image->blue_mask;
    for (int x = 0; x < info.width; x++)
        for (int y = 0; y < info.height; y++)
        {
            unsigned long pixel = XGetPixel(image, x, y);
            uint8_t blue = pixel & blue_mask;
            uint8_t green = (pixel & green_mask) >> 8;
            uint8_t red = (pixel & red_mask) >> 16;
            buffer[(x + info.width * y) * 3] = red;
            buffer[(x + info.width * y) * 3 + 1] = green;
            buffer[(x + info.width * y) * 3 + 2] = blue;
        }
    XDestroyImage(image);
}
