#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "Camera.hpp"
#include "Robot.hpp"
#include "Solver.hpp"
#include "esp_camera.h"
#include "esp_jpg_decode.h"
#include "img_converters.h"

const char* ssid = "CubeSolver";
const char* password = "#cubeSolver";

Robot* robot = nullptr;

WebServer server(80);

// Função de debug para ver a camara
void handleCapture() {
    analogWrite(LED_PIN, LED_BRIGHTNESS);
    delay(500);
    analogWrite(LED_PIN, 0);
    delay(LED_BRIGHTNESS_DELAY);
    // Obter frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        server.send(500, "text/plain", "Erro ao capturar imagem");
        return;
    }

    // transformar em jpeg
    uint8_t* jpeg_buf = nullptr;
    size_t jpeg_len = 0;
    int size = (fb->height - (fb->height / 10)) / 3;
    int cube_padding = size / 5;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int posx = j * size + cube_padding;
            int posy = i * size + cube_padding/2;
            Camera::draw(fb, posx, posy, size);
        }
    }
    bool success = fmt2jpg(fb->buf, fb->len, fb->width, fb->height,
                           PIXFORMAT_RGB565, 80, &jpeg_buf, &jpeg_len);

    // Enviar para o cliente
    WiFiClient client = server.client();
    if (!client || !success) {
        esp_camera_fb_return(fb);
        return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/jpeg");
    client.print("Content-Length: ");
    client.println(jpeg_len);
    client.println();
    client.write(jpeg_buf, jpeg_len);
    client.stop();

    free(jpeg_buf);

    esp_camera_fb_return(fb);
}

// Envia ficheiro do LittleFS para o cliente
void serveFile(const String& path, const String& contentType) {
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(404, "text/plain", "Ficheiro não encontrado");
    }
}

// Inicia todas as rotas
void setupRoutes() {
    // Principal
    server.on("/", HTTP_GET, []() { serveFile("/index.html", "text/html"); });

    // Envia CSS
    server.on("/css/style.css", HTTP_GET,
              []() { serveFile("/css/style.css", "text/css"); });

    // Envia JS
    server.on("/js/css3.oz.js", HTTP_GET,
              []() { serveFile("/js/css3.oz.js", "application/javascript"); });
    server.on("/js/oz.js", HTTP_GET,
              []() { serveFile("/js/oz.js", "application/javascript"); });
    server.on("/js/quaternion.js", HTTP_GET, []() {
        serveFile("/js/quaternion.js", "application/javascript");
    });
    server.on("/js/rubik.js", HTTP_GET,
              []() { serveFile("/js/rubik.js", "application/javascript"); });
    server.on("/js/custom.js", HTTP_GET,
              []() { serveFile("/js/custom.js", "application/javascript"); });

    // Envia o estado do cubo (peças)
    server.on("/cubestate", HTTP_GET, []() {
        auto f1 = robot->cube->piece_state();
        String cube_s = getCubeStateString(f1);
        server.send(200, "application/json", cube_s);
    });

    // Recebe uma sequencia de movimentos e executa-a
    server.on("/sendmove", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String data = server.arg("plain");
            robot->update(string(data.c_str()));
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Erro: Nenhum dado recebido");
        }
    });

    // Resolve o cubo e envia a resolução
    server.on("/solve", HTTP_GET, []() {
        server.send(200, "text/plain", String(robot->solve().c_str()));
    });

    // Aplica um scramble e envia os movimentos
    server.on("/scramble", HTTP_GET, []() {
        server.send(200, "text/plain", String(robot->scramble(20).c_str()));
    });

    server.on("/capture", HTTP_GET, handleCapture);

    server.on("/update_state", HTTP_POST, []() {
        if (server.hasArg("plain")) {
            String data = server.arg("plain");
            // Chama o método update_state com o novo estado
            robot->update_state(data.c_str());
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Erro: Nenhum dado recebido");
        }
    });

    // Le o cubo com a camera e envia o novo estado
    server.on("/scan", HTTP_GET, []() {
        robot->get_faces();
        auto f1 = robot->cube->piece_state();
        String cube_s = getCubeStateString(f1);
        server.send(200, "application/json", cube_s);
    });

    // Aplica um reset ao Cubo
    server.on("/reset", HTTP_POST, []() {
        robot->reset();
        server.send(200, "text/plain", "OK");
    });
}

// Transforma um array em uma string para ser lido pelo js
String getCubeStateString(const std::array<std::array<int, 3>, 26>& cubeState) {
    String result = "[";
    for (size_t j = 0; j < cubeState.size(); j++) {
        // Juntar trios de cores
        result += "[";
        for (int i = 0; i < 3; i++) {
            result += String(cubeState[j][i]);
            if (i < 2) result += ", ";
        }
        result += "]";
        // Adicionar a virgula para o próxima trio
        if (j < cubeState.size() - 1) result += ",";
    }
    result += "]";
    return result;
}

// Tenta gerar uma seed o mais próxima de aleatório
void randomSeed(){
    // Inicializar gerador de números aleatórios com seed única
    // Usar múltiplas fontes de aleatoriedade para garantir unicidade
    unsigned long seed = 0;

    // Usar tempo de inicialização (micros() para maior precisão)
    seed += micros();

    // Usar endereço MAC do WiFi
    uint8_t mac[6];
    WiFi.macAddress(mac);
    for (int i = 0; i < 6; i++) {
        seed = seed * 31 + mac[i];
    }

    // Usar ID único do chip ESP32
    seed += ESP.getEfuseMac();

    // Usar tempo de boot (em milissegundos)
    seed += millis();

    // Aplicar a seed
    srand(seed);

    Serial.printf("Seed inicializada: %lu\n", seed);
}

void setup() {
    // Iniciar o Serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nIniciando ...");


    // Iniciar o LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Falha ao montar LittleFS! A formatar...");
        LittleFS.format();
        if (!LittleFS.begin()) {
            Serial.println("Erro: Não foi possível montar LittleFS!");
            return;
        }
    }

    // Configurar Wi-Fi
    WiFi.softAP(ssid, password);

    // Informação sobre o Wi-Fi
    Serial.println("\nRede Wi-Fi criada!");
    Serial.print("Endereço IP do AP: ");
    Serial.println(WiFi.softAPIP());

    robot = new Robot();

    // Iniciar rotas
    setupRoutes();

    // Iniciar rotas
    server.begin();

    // Mensagem Final
    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
    // Receber Clientes
    server.handleClient();
    // Executar robô
    robot->run();
}