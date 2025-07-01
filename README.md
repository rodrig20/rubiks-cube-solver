# Rubik's Cube Solver

Um sistema robótico completo para resolver automaticamente um Cubo de Rubik, controlado por um microcontrolador ESP32-CAM. Este projeto combina hardware robótico (motores, câmera) com software de resolução e uma interface web interativa para visualização e controlo.

## Funcionalidades

- **Resolução Automática**: Algoritmo ZZ para resolver qualquer configuração do Cubo de Rubik
- **Captura de Imagem**: Câmara ESP32-CAM para detetar automaticamente o estado do cubo
- **Controlo Robótico**: Sistema de motores para executar movimentos físicos no cubo
- **Interface Web**: Visualização 3D interativa e controlo remoto via navegador
- **Baralhamento**: Função para gerar configurações aleatórias

## Requisitos de Hardware

### Componentes Principais
- **Microcontrolador**: ESP32 com suporte a câmera (ESP32-CAM ou similar)
- **Câmara**: Módulo ESP32-CAM para deteção de cores
- **Motores**:
  - Motor de base para rotação do cubo
  - Motor de garra para segurar e girar faces
- **Estrutura Mecânica**: Suporte para o cubo, motores e câmara
- **LED**: Iluminação para captura de imagem (embutido no ESP32-CAM)

### Conectividade
- Rede Wi-Fi para comunicação com interface web

## Dependências de Software

### Arduino IDE
- Testatado no Arduino IDE 2.3.4
- Pacote de suporte para placas ESP32
- Bibliotecas necessárias:
  - `ArduinoJson` - Parsing de JSON
  - `LittleFS` - Sistema de ficheiros
  - `WiFi` - Conectividade Wi-Fi (ESP32)
  - `WebServer` - Servidor web (ESP32)
  - `esp_camera` - Controlo da câmera ESP32-CAM

### Ferramentas Python (opcional)
- Python 3.7+
- Dependências em `tools/requirements.txt`:
  - `beautifulsoup4` - Web scraping
  - `requests` - Requisições HTTP
  - `RubikTwoPhase` - Algoritmo de resolução

## Instalação

1. Instalar o Arduino IDE e o pacote de suporte para placas necessário
2. Instalar as bibliotecas necessárias através do Gestor de Bibliotecas do Arduino
3. Fazer clone deste repositório ou descarregar o código fonte
4. Carregar os ficheiros do sistema de ficheiros para o ESP32 utilizando a ferramenta de carregamento LittleFS
5. Compilar e carregar o sketch principal para a placa

## Utilização

### 1. Inicialização do Sistema
1. Ligar a placa ESP32
2. Aguardar inicialização (ver mensagens no Monitor Série)
3. Conectar à rede Wi-Fi "CubeSolver" (password: "#cubeSolver")
4. Notar o endereço IP mostrado no Monitor Série

### 2. Interface Web
1. Abrir navegador e aceder ao IP do ESP32 (ex: `192.168.4.1`)
2. A interface 3D do cubo será carregada
3. Utilizar controlos para:
   - Visualizar o cubo em 3D
   - Executar movimentos manuais
   - Capturar estado atual via câmara
   - Resolver automaticamente
   - Baralhar o cubo

### 3. API Endpoints
- `GET /cubestate` - Obter estado atual do cubo
- `POST /sendmove` - Executar sequência de movimentos
- `GET /solve` - Resolver o cubo e retornar solução
- `GET /scramble` - Baralhar o cubo
- `GET /capture` - Capturar imagem da câmera (debug)
- `POST /update_state` - Atualizar estado do cubo
- `GET /scan` - Faz o scan do estado do cubo via câmera

## Estrutura do Projeto

### Ficheiros Principais
- `rubiks-cube-solver.ino` - Sketch principal do Arduino
- `Robot.hpp/cpp` - Principal
- `Solver.hpp/cpp` - Algoritmo de resolução do cubo
- `Camera.hpp/cpp` - Controlo da câmara e processamento de imagem
- `Motor.hpp/cpp` - Classes base para controlo de motores
- `BaseMotor.hpp/cpp` - Motor de rotação da base
- `GrabberMotor.hpp/cpp` - Motor da garra

### Interface Web (`data/`)
- `index.html` - Página principal
- `css/style.css` - Estilos da interface
- `js/` - Bibliotecas JavaScript:
  - `css3.oz.js`, `oz.js`, `quaternion.js`, `rubik.js` - Cubo virtual
  - `custom.js` - Integração com ESP32

### Ferramentas (`tools/`)
- `get_zbll.py` - Geração de algoritmos ZBLL
- `requirements.txt` - Dependências Python


### Rede Wi-Fi
- Modificar SSID e password em `rubiks-cube-solver.ino`
- Configurar IP estático se necessário

## Créditos
- **Cubo Virtual**: Baseada em [ondras/rubik](https://github.com/ondras/rubik)
