#include "Camera.hpp"

#include <Arduino.h>

#include <cmath>
#include <iostream>

#include "esp_camera.h"

// Troca a ordem de dois bytes
uint16_t byte_swap(uint16_t value) { return (value >> 8) | (value << 8); }

// Transforma uma cor RGB numa cor HSV
HSV rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b) {
    // Normaliza os valores RGB para o intervalo [0, 1]
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;

    // Encontra o valor máximo e mínimo entre R, G e B
    float max = fmaxf(fr, fmaxf(fg, fb));
    float min = fminf(fr, fminf(fg, fb));

    // Calcula a diferença entre o máximo e o mínimo (delta)
    float delta = max - min;

    // Inicializa o valor da matiz (hue)
    float h = 0.0f;

    // Calcula a matiz só se delta for diferente de zero (cor não cinzenta)
    if (delta != 0.0f) {
        // Determina a fórmula a usar consoante qual componente é o máximo
        if (max == fr) {
            h = fmodf(((fg - fb) / delta), 6.0f);
        } else if (max == fg) {
            h = ((fb - fr) / delta) + 2.0f;
        } else {
            h = ((fr - fg) / delta) + 4.0f;
        }
        // Converte o resultado para graus
        h *= 60.0f;
        // Garante que a matiz é sempre positiva
        if (h < 0.0f) h += 360.0f;
    }

    // Calcula a saturação: zero se máximo for zero (preto), senão a proporção
    // delta/max
    float s = (max == 0.0f) ? 0.0f : delta / max;
    // O valor (brilho) é o valor máximo entre R, G e B normalizado
    float v = max;

    return {h, s, v};
}

float distance(Color a, Color b) {
    HSV ha = rgb_to_hsv(a.R, a.G, a.B);
    HSV hb = rgb_to_hsv(b.R, b.G, b.B);

    // Diferença de hue
    float dh = fabsf(ha.H - hb.H);
    if (dh > 180.0f) dh = 360.0f - dh;

    float ds = fabsf(ha.S - hb.S);
    float dv = fabsf(ha.V - hb.V);

    // Se saturação for muito baixa, a cor é cinzenta/branca/preta: ignora Hue
    bool low_sat_a = ha.S < 0.2f;
    bool low_sat_b = hb.S < 0.2f;

    float peso_h = 1.0f;
    float peso_s = 0.5f;
    float peso_v = 0.5f;

    if (low_sat_a && low_sat_b) {
        peso_h = 0.0f;  // cor sem hue relevante
        peso_s = 0.2f;
        peso_v = 1.5f;  // brilho é o que mais conta
    }

    return peso_h * dh + peso_s * ds * 100.0f + peso_v * dv * 100.0f;
}

// Contrutor
Camera::Camera() { startCamera(); }

// Faz a conversão de RGB565 para RGB
void Camera::rgb565ToRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
    // RGB565: R = 5 bits, G = 6 bits, B = 5 bits
    r = ((color >> 11) & 0x1F) << 3;  // 5 bits para 8 bits
    g = ((color >> 5) & 0x3F) << 2;   // 6 bits para 8 bits
    b = (color & 0x1F) << 3;          // 5 bits para 8 bits
}

// Obtem a cor média de uma peça
Color Camera::get_color_piece(camera_fb_t* fb, int posx, int posy, int size) {
    int piece_padding = size / 4;
    uint16_t* pixels = (uint16_t*)fb->buf;

    // Inicializar valores para media
    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;

    // Inicializar variaveis
    int x_start = posx + piece_padding;
    int y_start = posy + piece_padding;

    int x_end = posx + size - piece_padding;
    int y_end = posy + size - piece_padding;

    int stride_px = (fb->len / fb->height) / 2;
    int area = 0;

    // Percorrer area para a média de cor
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = y * fb->width + x;
            // Obter pixel invertido
            uint16_t pixel = byte_swap(pixels[idx]);

            uint8_t r;
            uint8_t g;
            uint8_t b;
            // Converter para RGB e adicionar para media
            rgb565ToRGB(pixel, r, g, b);
            r_sum += r;
            g_sum += g;
            b_sum += b;
            area++;
        }
    }
    // Retorna cor
    Color color = {(r_sum / area), (g_sum / area), (b_sum / area)};
    return color;
}

// Desenha a borda da área da média
void Camera::draw(camera_fb_t* fb, int posx, int posy, int size) {
    int piece_padding = size / 4;
    uint16_t* pixels = (uint16_t*)fb->buf;

    // Inicializar variaveis
    int x_start = posx + piece_padding;
    int y_start = posy + piece_padding;

    int x_end = posx + size - piece_padding;
    int y_end = posy + size - piece_padding;

    int stride_px = (fb->len / fb->height) / 2;

    // Percorrer area para a média de cor
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = y * fb->width + x;
            // Desenhar
            if (y < y_start + 2 || y >= y_end - 2 || x < x_start + 2 ||
                x >= x_end - 2) {
                uint16_t new_pixel = ~0;
                pixels[idx] = new_pixel;
            }
        }
    }
}

// Obtem a lista de cores de cores de uma face
Color* Camera::get_color_face() {
    // Liga o LED
    analogWrite(LED_PIN, LED_BRIGHTNESS);
    delay(500);
    analogWrite(LED_PIN, 0);
    delay(LED_BRIGHTNESS_DELAY);

    // Obtém a imagem
    camera_fb_t* fb = esp_camera_fb_get();
    int size = (fb->height - (fb->height / 10)) / 3;
    int cube_padding = size / 5;
    Color* cube_state = new Color[9];

    // Percorre as 9 peças do cubo para ler
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int posx = j * size + cube_padding;
            int posy = i * size + cube_padding / 2;
            cube_state[(i * 3) + j] = get_color_piece(fb, posx, posy, size);
        }
    }

    esp_camera_fb_return(fb);
    return cube_state;
}

// Obtem a lista de cores de cores de uma face (debug)
Color* Camera::get_color_face_debug(camera_fb_t* fb) {
    int size = (fb->height - (fb->height / 10)) / 3;
    int cube_padding = size / 5;
    Color* cube_state = new Color[9];

    // Percorre as 9 peças do cubo para ler
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int posx = j * size + cube_padding;
            int posy = i * size + cube_padding;
            cube_state[(i * 3) + j] = get_color_piece(fb, posx, posy, size);
        }
    }

    // Percorre as 9 peças do cubo para desenhar borda
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int posx = j * size;
            int posy = i * size;
            draw(fb, posx, posy, size);
        }
    }

    return cube_state;
}

// Inicializa a camara
void Camera::startCamera() {
    // Configuração da camara para Ai-Thinker
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_128X128;
    config.pixel_format = PIXFORMAT_RGB565;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    config.fb_count = 5;
    config.grab_mode = CAMERA_GRAB_LATEST;
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        printf("Erro ao iniciar a câmara: 0x%x", err);
        initialized = 0;
        return;
    }

    // Configuarar Sensor
    sensor_t* s = esp_camera_sensor_get();

    s->set_agc_gain(s, 0);                    // Desativa ganho automático
    s->set_gainceiling(s, (gainceiling_t)2);  // Garante pouco ganho

    s->set_aec2(s, 0);           // Desativa AEC2
    s->set_exposure_ctrl(s, 0);  // Controlo manual da exposição
    s->set_aec_value(s, 85);     // Define expoisção manualmente

    s->set_brightness(s, 1);  // Reduz brilho
    s->set_saturation(s, 3);  // Aumenta muito a saturação
    s->set_contrast(s, 2);    // Contraste normal

    // Pin do LED
    pinMode(LED_PIN, OUTPUT);

    initialized = 1;
}

// Agrupa as cores em 6 grupos
void Camera::grouping_colors(Color cores[54], int labels[54]) {
    int idxs_ref[6] = {4, 13, 22, 31, 40, 49};  // centros
    int grup_size[6] = {1, 1, 1, 1, 1, 1};      // tamanho do grupo
    float distances[6][54] = {};  // distancias dos centros a cada peça
    // Número da peça que está mais próximo de cada centro
    int lowest_dist_center_idx[6] = {};
    // Distancia entre o centro e a peça indicada a cima
    float lowest_dist_center[6] = {};

    // Reset ao label
    for (int i = 0; i < 54; i++) {
        labels[i] = -1;
    }

    // Percorrer centros
    for (int i = 0; i < 6; i++) {
        labels[idxs_ref[i]] = i;
        lowest_dist_center[i] = MAXFLOAT;
        lowest_dist_center_idx[i] = -1;
        // Obter todas as distancias
        for (int j = 0; j < 54; j++) {
            if (idxs_ref[i] != j) {
                distances[i][j] = distance(cores[idxs_ref[i]], cores[j]);
            } else {
                distances[i][j] = MAXFLOAT;
            }
        }
    }

    // Obter todas a peça mais próxima de cada centro
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 54; j++) {
            int is_ref = 0;
            // Se a peça fro ela própria ou ouro centro
            for (int k = 0; k < 6; k++) {
                if (j == idxs_ref[k]) {
                    is_ref = 1;
                    break;
                };
            }
            if (!is_ref) {
                if (distances[i][j] < lowest_dist_center[i]) {
                    lowest_dist_center[i] = distances[i][j];
                    lowest_dist_center_idx[i] = j;
                }
            }
        }
    }

    // Executar o número de peças existentes exceto os centros
    for (int i = 0; i < 54 - 6; i++) {
        int lowest_idx = -1;

        // Verificar qual dos centros tem a peça mais próxima
        for (int j = 0; j < 6; j++) {
            if (grup_size[j] >= 9) continue; // tamnho máximo é 9
            if (lowest_idx == -1) {
                lowest_idx = j;
            }
            if (lowest_dist_center[j] < lowest_dist_center[lowest_idx]) {
                lowest_idx = j;
            }
        }
        // Alterar o label
        int idx = lowest_dist_center_idx[lowest_idx];
        labels[idx] = lowest_idx;
        grup_size[lowest_idx]++;

        // Atualizar lista de peças mais próxima de cada centro
        for (int k = 0; k < 6; k++) {
            distances[k][idx] = MAXFLOAT;
            if (lowest_dist_center_idx[k] == idx) {
                // Reset às variaveis
                lowest_dist_center[k] = MAXFLOAT;
                lowest_dist_center_idx[k] = -1;

                for (int j = 0; j < 54; j++) {
                    int is_ref = 0;
                    // Não pode ser centro
                    for (int l = 0; l < 6; l++) {
                        if (j == idxs_ref[l]) {
                            is_ref = 1;
                            break;
                        };
                    }
                    // Não pode ter valor atribuido nem ser centro
                    if (labels[j] == -1 && !is_ref) {
                        if (distances[k][j] < lowest_dist_center[k]) {
                            lowest_dist_center[k] = distances[k][j];
                            lowest_dist_center_idx[k] = j;
                        }
                    }
                }
            }
        }
    }
}
