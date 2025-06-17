#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "esp_camera.h"

// Pinos da câmara (modelo AI-Thinker ESP32-CAM com OV2640)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

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
    void draw(camera_fb_t* fb, int posx, int posy, int size);
    void startCamera();
    void capture();
};

#endif