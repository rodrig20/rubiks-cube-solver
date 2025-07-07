#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>

#include "src/Robot.hpp"

Robot* robot = nullptr;

// Tenta gerar uma seed o mais próxima de aleatório
void randomSeed() {
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

    randomSeed();

    // Iniciar o LittleFS
    if (!LittleFS.begin()) {
        Serial.println("Falha ao montar LittleFS! A formatar...");
        LittleFS.format();
        if (!LittleFS.begin()) {
            Serial.println("Erro: Não foi possível montar LittleFS!");
            return;
        }
    }

    robot = new Robot();

    // Mensagem Final
    Serial.println("Servidor HTTP iniciado!");
}

void loop() {
    // Executar robô
    robot->run();
}
