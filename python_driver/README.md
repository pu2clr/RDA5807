# Driver RDA5807 Python

Este é um driver Python que replica a interface da biblioteca Arduino RDA5807 em C/C++, permitindo controlar o rádio DSP RDA5807 via Python.

## 🎯 Características

- **Interface idêntica** à biblioteca C/C++ Arduino
- **Modo simulação** para testes sem hardware
- **Comunicação I2C** real com o chip RDA5807
- **Controle via MQTT** para IoT
- **Interface Web** para controle remoto
- **Exemplos completos** de uso

## 📁 Arquivos

- `rda5807.py` - Driver principal com todas as funções
- `exemplo_uso.py` - Exemplos de uso e rádio interativo
- `servidor_web.py` - Interface web para controle via navegador
- `mqtt_client.py` - Cliente MQTT para controle remoto
- `requirements.txt` - Dependências opcionais

## 🚀 Instalação

### Básico (apenas simulação)
```bash
# Não requer dependências externas
python3 rda5807.py
```

### Com I2C (hardware real - Raspberry Pi)
```bash
sudo apt-get install python3-smbus
pip3 install smbus
```

### Com MQTT
```bash
pip3 install paho-mqtt
```

### Completo (todas as funcionalidades)
```bash
sudo apt-get install python3-smbus  # Raspberry Pi/Ubuntu
pip3 install smbus paho-mqtt flask flask-socketio
```

## 💻 Uso Básico

```python
import rda5807

# Criar instância do rádio
rx = rda5807.RDA5807()

# Inicializar
rx.setup()

# Configurar frequência e volume
rx.setFrequency(10390)  # 103.9 MHz
rx.setVolume(8)

# Controles
rx.setFrequencyUp()    # Subir frequência
rx.setVolumeDown()     # Diminuir volume
rx.setMute(True)       # Ligar mudo
rx.setBass(True)       # Ligar bass boost

# Obter status
freq = rx.getFrequency()
vol = rx.getVolume()
status = rx.getStatus()
```

## 🎵 Principais Funções

### Controle de Frequência
- `setFrequency(freq)` - Define frequência (10390 = 103.9 MHz)
- `getFrequency()` - Obtém frequência atual
- `setFrequencyUp()` / `setFrequencyDown()` - Incrementa/decrementa
- `seek(mode, direction)` - Busca automática de estações

### Controle de Volume
- `setVolume(0-15)` - Define volume
- `getVolume()` - Obtém volume
- `setVolumeUp()` / `setVolumeDown()` - Incrementa/decrementa
- `setMute(bool)` / `isMuted()` - Controle de mudo

### Configurações de Áudio
- `setBass(bool)` / `getBass()` - Bass boost
- `setMono(bool)` / `isStereo()` - Mono/Estéreo
- `setRDS(bool)` - Liga/desliga RDS

### Controle de Banda
- `setBand(band)` - Define banda FM
- `getBand()` - Obtém banda atual
- Constantes: `RDA_FM_BAND_USA_EU`, `RDA_FM_BAND_WORLD`, etc.

## 🔧 Exemplos de Uso

### 1. Teste Básico
```bash
python3 rda5807.py
```

### 2. Exemplos Interativos
```bash
python3 exemplo_uso.py
```

### 3. Interface Web
```bash
python3 servidor_web.py
# Acesse: http://localhost:8080
```

### 4. Controle via MQTT
```bash
# Iniciar cliente MQTT
python3 mqtt_client.py

# Comandos via mosquitto:
mosquitto_pub -h localhost -t /home/RDA5807/frequency -m "103.9"
mosquitto_pub -h localhost -t /home/RDA5807/volume -m "10"
mosquitto_pub -h localhost -t /home/RDA5807/command -m "seek_up"
```

## 📡 Tópicos MQTT

- `/home/RDA5807/frequency` - Controle de frequência
- `/home/RDA5807/volume` - Controle de volume
- `/home/RDA5807/command` - Comandos gerais
- `/home/RDA5807/status` - Status do rádio

### Comandos MQTT Suportados

**Frequência:**
- `103.9` - Define frequência em MHz
- `up` / `+` - Incrementa frequência
- `down` / `-` - Decrementa frequência

**Volume:**
- `8` - Define volume (0-15)
- `up` / `+` - Incrementa volume
- `down` / `-` - Decrementa volume

**Comandos Gerais:**
- `mute_toggle` - Liga/desliga mudo
- `bass_toggle` - Liga/desliga bass boost
- `seek_up` / `seek_down` - Busca estações
- `status` - Solicita status atual
- `reset` - Reinicia rádio

## 🔌 Hardware

### Conexão I2C
```
RDA5807    Raspberry Pi
VCC   -->  3.3V
GND   -->  GND
SDA   -->  GPIO 2 (SDA)
SCL   -->  GPIO 3 (SCL)
```

### Endereços I2C
- `0x10` - Acesso completo (padrão)
- `0x11` - Acesso direto a registradores

## 📊 Interface Web

A interface web oferece:
- 🎛️ Controles de frequência e volume
- 📻 Seleção de banda FM
- 🔊 Controles de áudio (mudo, bass, mono/estéreo)
- 🔍 Busca automática de estações
- 📊 Status em tempo real
- 📱 Design responsivo

## 🔄 Compatibilidade com C++

O driver mantém **compatibilidade total** com a biblioteca C++:

| Função C++ | Função Python | Descrição |
|------------|---------------|-----------|
| `rx.setup()` | `rx.setup()` | Inicialização |
| `rx.setFrequency(10390)` | `rx.setFrequency(10390)` | Define frequência |
| `rx.setVolume(8)` | `rx.setVolume(8)` | Define volume |
| `rx.setFrequencyUp()` | `rx.setFrequencyUp()` | Sobe frequência |
| `rx.seek(0, 1)` | `rx.seek(0, 1)` | Busca estação |
| `rx.isStereo()` | `rx.isStereo()` | Verifica estéreo |
| `rx.getFrequency()` | `rx.getFrequency()` | Obtém frequência |

## ⚠️ Limitações

- **I2C Hardware:** Requer Raspberry Pi ou similar para controle real
- **Simulação:** Modo padrão simula o comportamento para testes
- **RDS:** Implementação básica (expandível)

## 🎯 Casos de Uso

1. **Prototipagem rápida** - Testar funcionalidades sem hardware
2. **IoT Projects** - Controle remoto via MQTT/WiFi
3. **Automação residencial** - Integração com Home Assistant
4. **Interfaces web** - Controle via navegador
5. **Educação** - Aprendizado de conceitos de rádio DSP

## 🚀 Próximos Recursos

- [ ] Interface gráfica (Tkinter/PyQt)
- [ ] Suporte a multiple RDA5807
- [ ] RDS completo com decodificação
- [ ] Gravação de presets
- [ ] API RESTful
- [ ] WebSocket para tempo real
- [ ] Integração com Spotify/Internet Radio

## 📝 Exemplos Adicionais

### Controle via Terminal
```python
rx = rda5807.RDA5807()
rx.setup()

# Loop de controle
while True:
    cmd = input("Comando: ")
    if cmd == 'u': rx.setFrequencyUp()
    elif cmd == 'd': rx.setFrequencyDown() 
    elif cmd == '+': rx.setVolumeUp()
    elif cmd == '-': rx.setVolumeDown()
    elif cmd == 's': rx.seek(0, 1)
    elif cmd == 'q': break
    print(f"Freq: {rx.getFrequency()/100:.1f} MHz, Vol: {rx.getVolume()}")
```

### Monitoramento RSSI
```python
import time

rx = rda5807.RDA5807()
rx.setup()

while True:
    rssi = rx.getRssi()
    freq = rx.getFrequency() / 100
    print(f"Freq: {freq:.1f} MHz, RSSI: {rssi}")
    time.sleep(1)
```

## 🤝 Contribuições

Contribuições são bem-vindas! Este driver foi criado baseado na excelente biblioteca C++ de **Ricardo Lima Caratti (PU2CLR)**.

## 📄 Licença

MIT License - Baseado no trabalho original de Ricardo Lima Caratti.

---

**Desenvolvido como extensão Python da biblioteca RDA5807 Arduino de Ricardo Lima Caratti (PU2CLR)**