#include <Arduino.h>

// Incluir a biblioteca correta com base na plataforma
#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#endif
  #include <LittleFS.h>

#include "Solver.hpp"

const char* ssid = "CubeSolver";
const char* password = "#cubeSolver"; // Pelo menos 8 caracteres obrigatórios no 

Solver cube("054305225013310135223124124533035111423144040402550245");
#ifdef ESP32
WebServer server(80);
#else
Esp8266WebServer server(80);
#endif


void serveFile(const String& path, const String& contentType) {
  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "Ficheiro não encontrado");
  }
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    serveFile("/index.html", "text/html");
  });

  server.on("/css/style.css", HTTP_GET, []() {
    serveFile("/css/style.css", "text/css");
  });

  server.on("/js/app.js", HTTP_GET, []() {
    serveFile("/js/app.js", "application/javascript");
  });

  server.on("/cubestate", HTTP_GET, []() {
    server.send(200, "application/json", getCubeStateString(cube.piece_state()));
  });

  server.on("/sendmove", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String data = server.arg("plain");
      cube.move(string(data.c_str()));
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Erro: Nenhum dado recebido");
    }
  });

  server.on("/solve", HTTP_GET, []() {
    server.send(200, "text/plain", String(cube.solve().c_str()));
  });

  server.on("/scramble", HTTP_GET, []() {
    server.send(200, "text/plain", String(cube.scramble(26).c_str()));
  });
}

String getCubeStateString(const std::array<std::array<int, 3>, 26>& cubeState) {
  String result = "[";
  for (size_t j = 0; j < cubeState.size(); j++) {
    result += "[";
    for (int i = 0; i < 3; i++) {
      result += String(cubeState[j][i]);
      if (i < 2) result += ", ";
    }
    result += "]";
    if (j < cubeState.size() - 1) result += ",";
  }
  result += "]";
  return result;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nIniciando ...");

  if (!LittleFS.begin()) {
    Serial.println("Falha ao montar LittleFS! A formatar...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("Erro: Não foi possível montar LittleFS!");
      return;
    }
  }

  // Configurar Wi-Fi dependendo da plataforma
  WiFi.softAP(ssid, password);
  
  Serial.println("\nRede Wi-Fi criada!");
  Serial.print("Endereço IP do AP: ");
  Serial.println(WiFi.softAPIP());

  setupRoutes();

  server.begin();
  Serial.println("Servidor HTTP iniciado!");

 //Solver cube_p(Solver::solved_string());
  //cube_p.move("U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2");

  //cube.match_state(cube_p);
}

void loop() {
  server.handleClient();
}
