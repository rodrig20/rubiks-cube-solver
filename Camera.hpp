#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "esp_camera.h"

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} Color;

class Camera {
   public:
    Camera();
    void rgb565ToRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b);
    Color get_color_piece(camera_fb_t* fb, int posx, int posy, int size);
    Color* get_color_face(camera_fb_t* fb);
};

#endif