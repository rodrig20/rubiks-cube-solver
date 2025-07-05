import serial
import time

# Configurações
serial_port = '/dev/ttyUSB0'
baud_rate = 115200
ficheiro_saida = "/".join(__file__.split("/")[:-1])+'/serial.log'

# Abre a serial port
try:
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    print(f'[INFO] Ligado a {serial_port} @ {baud_rate} baud')
except serial.SerialException:
    print(f'[ERRO] Não foi possível abrir a porta {serial_port}')
    exit(1)

# Aguarda ligação
time.sleep(2)

# Abre ficheiro para escrita
with open(ficheiro_saida, 'w') as f:
    print(f'[INFO] A guardar dados em: {ficheiro_saida}')
    try:
        while True:
            linha = ser.readline().decode('utf-8', errors='ignore').strip()
            # Escreve a linha
            if linha:
                timestamp = time.strftime('%H:%M:%S')
                f.write(f'[{timestamp}] {linha}\n')
                f.flush()
                print(f'[{timestamp}] {linha}')
    except KeyboardInterrupt:
        print('\n[INFO] Parado pelo utilizador (Ctrl+C).')
    finally:
        ser.close()
        print('[INFO] Porta série fechada.')
