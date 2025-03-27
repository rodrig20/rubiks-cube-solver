# Rubik's Cube Solver

Um script com uma interface web interativa para manipular um Cubo de Rubik, executada em microcontroladores ESP32. Este projeto fornece uma visualização 3D de um Cubo de Rubik que pode ser manipulado, embaralhado e resolvido automaticamente.

## Funcionalidades

- Visualização 3D interativa do Cubo de Rubik
- Controlos para rotação do cubo e manipulação das camadas
- Capacidade de resolução automática
- Função de embaralhar
- Suporte para introdução manual de movimentos
- Acesso wireless através de rede Wi-Fi criada pelo ESP32

## Requisitos de Hardware

- Microcontrolador ESP32
- Computador ou dispositivo móvel com capacidade Wi-Fi

## Dependências de Software

- Arduino IDE
- Pacote de suporte para placas ESP32
- Bibliotecas Arduino necessárias:
  - ArduinoJson
  - LittleFS
  - Wi-Fi (ESP32)
  - WebServer (ESP32)

## Instalação

1. Instalar o Arduino IDE e o pacote de suporte para placas necessário
2. Instalar as bibliotecas necessárias através do Gestor de Bibliotecas do Arduino
3. Fazer clone deste repositório ou descarregar o código fonte
4. Carregar os ficheiros do sistema de ficheiros para o ESP32 utilizando a ferramenta de carregamento LittleFS
5. Compilar e carregar o sketch principal para a placa

## Utilização

1. Ligar a placa ESP32
2. Ligar à rede Wi-Fi denominada "CubeSolver" (palavra-passe default: "#cubeSolver")
3. Abrir um navegador web e aceder ao endereço IP mostrado no Monitor Série (tipicamente 192.168.4.1)
4. A interface do Cuboserá carregada no navegador


## Estrutura do Projeto

- `rubiks-cube-solver.ino`: Ficheiro principal do Arduino
- `Solver.hpp/cpp`: Implementação em C++ do resolvedor do Cubo de Rubik
- `data/`: Ficheiros da interface web
  - `index.html`: Página web principal
  - `css/style.css`: Estilos
  - `js/app.js`: Lógica de visualização e interação 3D do cubo
  - `info/EO.txt`: Algoritmos pré computados para orientação das edges do cubo

## Créditos

- Visualização 3D do Cubo de Rubik baseada em [ondras/rubik](https://github.com/ondras/rubik)