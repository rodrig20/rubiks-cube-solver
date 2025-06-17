#include <Arduino.h>

#include <WebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "esp_camera.h"
#include "esp_jpg_decode.h"
#include "img_converters.h"


#include "Solver.hpp"
#include "Camera.hpp"
#include "Robot.hpp"

const char* ssid = "CubeSolver";
const char* password = "#cubeSolver";


Solver cube("054305225013310135223124124533035111423144040402550245");
Camera cam = Camera();

WebServer server(80);

const int LED_PIN = 4;

void handleCapture() {
  for (int i = 0; i < 6; ++i) {
      auto fb_temp = esp_camera_fb_get();
      esp_camera_fb_return(fb_temp);
  }
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Erro ao capturar imagem");
    return;
  }

  Color* cube_state = cam.get_color_face(fb);
  Serial.println("=================");
  for (int i = 0; i < 9; i++) {
    Serial.printf("[%d, %d, %d]\n", cube_state[i].R, cube_state[i].G, cube_state[i].B);
  }

  delete[] cube_state;
  cube_state = nullptr;

  uint8_t* jpeg_buf = nullptr;
  size_t jpeg_len = 0;

  bool success = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 80, &jpeg_buf, &jpeg_len);
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
  server.on("/", HTTP_GET, []() {
    serveFile("/index.html", "text/html");
  });

  // Envia CSS
  server.on("/css/style.css", HTTP_GET, []() {
    serveFile("/css/style.css", "text/css");
  });

  // Envia JS
  server.on("/js/app.js", HTTP_GET, []() {
    serveFile("/js/app.js", "application/javascript");
  });

  // Envia o estado do cubo (peças)
  server.on("/cubestate", HTTP_GET, []() {
    server.send(200, "application/json",
                getCubeStateString(cube.piece_state()));
  });

  // Recebe uma sequencia de movimentos e executa-a
  server.on("/sendmove", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String data = server.arg("plain");
      cube.move(string(data.c_str()));
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Erro: Nenhum dado recebido");
    }
  });

  // Resolve o cubo e envia a resolução
  server.on("/solve", HTTP_GET, []() {
    server.send(200, "text/plain", String(cube.solve().c_str()));
  });

  // Aplica um scramble e envia os movimentos
  server.on("/scramble", HTTP_GET, []() {
    server.send(200, "text/plain", String(cube.scramble(20).c_str()));
  });

  server.on("/capture", HTTP_GET, handleCapture);
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

  cam.startCamera();

  // Iniciar rotas
  setupRoutes();

  // Iniciar rotas
  server.begin();

  // Mensagem Final
  Serial.println("Servidor HTTP iniciado!");

  Robot r = Robot();
  r.move("R2 L2 F2 B2 U2 D2");

}

void loop() {
  // Receber Clientes
  server.handleClient();
}
