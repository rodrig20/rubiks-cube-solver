#include "CubeServer.hpp"

#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include <iostream>

#include "Robot.hpp"
#include "esp_jpg_decode.h"
#include "img_converters.h"

// Função de debug para ver a camara
void CubeServer::handleCapture() {
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
            int posy = i * size + cube_padding / 2;
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
void CubeServer::serveFile(const String& path, const String& contentType) {
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(404, "text/plain", "Ficheiro não encontrado");
    }
}

// Inicia todas as rotas
void CubeServer::setupRoutes() {
    // Principal
    server.on("/", HTTP_GET,
              [this]() { serveFile("/index.html", "text/html"); });

    // Envia CSS
    server.on("/css/style.css", HTTP_GET,
              [this]() { serveFile("/css/style.css", "text/css"); });

    // Envia JS
    server.on("/js/css3.oz.js", HTTP_GET, [this]() {
        serveFile("/js/css3.oz.js", "application/javascript");
    });
    server.on("/js/oz.js", HTTP_GET,
              [this]() { serveFile("/js/oz.js", "application/javascript"); });
    server.on("/js/quaternion.js", HTTP_GET, [this]() {
        serveFile("/js/quaternion.js", "application/javascript");
    });
    server.on("/js/rubik.js", HTTP_GET, [this]() {
        serveFile("/js/rubik.js", "application/javascript");
    });
    server.on("/js/custom.js", HTTP_GET, [this]() {
        serveFile("/js/custom.js", "application/javascript");
    });

    // Envia o estado do cubo (peças)
    server.on("/cubestate", HTTP_GET, [this]() {
        if (robot->has_state) {
            auto f1 = robot->cube->piece_state();
            String cube_s = getCubeStateString(f1);
            server.send(200, "application/json", cube_s);
        } else {
            server.send(200, "application/json", String(""));
        }
    });

    // Recebe uma sequencia de movimentos e executa-a
    server.on("/sendmove", HTTP_POST, [this]() {
        if (server.hasArg("plain")) {
            String data = server.arg("plain");
            robot->update(string(data.c_str()));
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Erro: Nenhum dado recebido");
        }
    });

    // Resolve o cubo e envia a resolução
    server.on("/solve", HTTP_GET, [this]() {
        server.send(200, "text/plain", String(robot->solve().c_str()));
    });

    // Aplica um scramble e envia os movimentos
    server.on("/scramble", HTTP_GET, [this]() {
        server.send(200, "text/plain", String(robot->scramble(20).c_str()));
    });

    server.on("/capture", HTTP_GET, [this]() { handleCapture(); });

    server.on("/update_state", HTTP_POST, [this]() {
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
    server.on("/scan", HTTP_GET, [this]() {
        robot->get_faces();
        server.send(200, "text/plain", "OK");
    });

    // Aplica um reset ao Cubo
    server.on("/reset", HTTP_POST, [this]() {
        robot->reset();
        server.send(200, "text/plain", "OK");
    });
}

// Transforma um array em uma string para ser lido pelo js
String CubeServer::getCubeStateString(
    const std::array<std::array<int, 3>, 26>& cubeState) {
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

CubeServer::CubeServer(Robot* robot_ref, int port) : server(port) {
    robot = robot_ref;

    // Configurar Wi-Fi
    WiFi.softAP(ssid, password);

    // Informação sobre o Wi-Fi
    std::cout << "\nRede Wi-Fi criada!\nEndereço IP do AP: ";
    std::cout << WiFi.softAPIP() << std::endl;

    setupRoutes();
    server.begin();
}

void CubeServer::handleClient() { server.handleClient(); }

CubeServer::~CubeServer() { delete robot; }
