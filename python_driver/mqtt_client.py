#!/usr/bin/env python3
"""
Cliente MQTT para controle remoto do RDA5807
Permite controlar o rádio via mensagens MQTT
"""

import sys
import os
import json
import time
import threading

# Adicionar o diretório do driver ao path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import rda5807

try:
    import paho.mqtt.client as mqtt
    MQTT_AVAILABLE = True
except ImportError:
    MQTT_AVAILABLE = False
    print("Biblioteca paho-mqtt não encontrada.")
    print("Instale com: pip install paho-mqtt")


class RDA5807MQTTClient:
    """Cliente MQTT para controle do RDA5807"""
    
    def __init__(self, broker_host="localhost", broker_port=1883, simulate=True):
        """
        Inicializa o cliente MQTT
        
        Args:
            broker_host: Endereço do broker MQTT
            broker_port: Porta do broker MQTT  
            simulate: Se True, executa em modo simulação
        """
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.simulate = simulate
        
        # Tópicos MQTT
        self.topic_frequency = "/home/RDA5807/frequency"
        self.topic_volume = "/home/RDA5807/volume"
        self.topic_command = "/home/RDA5807/command"
        self.topic_status = "/home/RDA5807/status"
        
        # Inicializar rádio
        self.radio = rda5807.RDA5807(simulate=simulate)
        self.radio.setup()
        self.radio.setFrequency(10390)  # 103.9 MHz padrão
        self.radio.setVolume(8)
        
        # Cliente MQTT
        if MQTT_AVAILABLE:
            self.client = mqtt.Client()
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message
            self.client.on_disconnect = self.on_disconnect
        else:
            self.client = None
        
        self.connected = False
        self.running = False
    
    def on_connect(self, client, userdata, flags, rc):
        """Callback quando conecta ao broker MQTT"""
        if rc == 0:
            self.connected = True
            print(f"✅ Conectado ao broker MQTT {self.broker_host}:{self.broker_port}")
            
            # Assinar tópicos
            topics = [
                (self.topic_frequency, 0),
                (self.topic_volume, 0), 
                (self.topic_command, 0)
            ]
            
            for topic, qos in topics:
                client.subscribe(topic, qos)
                print(f"📡 Assinando tópico: {topic}")
            
            # Publicar status inicial
            self.publish_status()
            
        else:
            print(f"❌ Erro ao conectar ao broker MQTT. Código: {rc}")
            self.connected = False
    
    def on_disconnect(self, client, userdata, rc):
        """Callback quando desconecta do broker MQTT"""
        self.connected = False
        print(f"🔌 Desconectado do broker MQTT. Código: {rc}")
    
    def on_message(self, client, userdata, msg):
        """Processa mensagens MQTT recebidas"""
        try:
            topic = msg.topic
            payload = msg.payload.decode('utf-8').strip()
            
            print(f"📨 Mensagem recebida - Tópico: {topic}, Payload: {payload}")
            
            if topic == self.topic_frequency:
                self.handle_frequency_command(payload)
            elif topic == self.topic_volume:
                self.handle_volume_command(payload)
            elif topic == self.topic_command:
                self.handle_general_command(payload)
            
            # Publicar status atualizado após comando
            time.sleep(0.1)  # Pequeno delay
            self.publish_status()
            
        except Exception as e:
            print(f"❌ Erro ao processar mensagem: {e}")
    
    def handle_frequency_command(self, payload):
        """Processa comandos de frequência"""
        try:
            if payload.lower() in ['up', '+']:
                self.radio.setFrequencyUp()
                print(f"🔺 Frequência aumentada para {self.radio.getFrequency()/100:.1f} MHz")
                
            elif payload.lower() in ['down', '-']:
                self.radio.setFrequencyDown()
                print(f"🔻 Frequência diminuída para {self.radio.getFrequency()/100:.1f} MHz")
                
            else:
                # Tentar interpretar como frequência em MHz ou kHz
                freq_value = float(payload)
                
                if freq_value < 200:  # Provavelmente MHz
                    freq_khz = int(freq_value * 100)
                else:  # Provavelmente em dezenas de kHz
                    freq_khz = int(freq_value)
                
                self.radio.setFrequency(freq_khz)
                print(f"📻 Frequência definida para {self.radio.getFrequency()/100:.1f} MHz")
                
        except ValueError:
            print(f"❌ Valor de frequência inválido: {payload}")
    
    def handle_volume_command(self, payload):
        """Processa comandos de volume"""
        try:
            if payload.lower() in ['up', '+']:
                self.radio.setVolumeUp()
                print(f"🔊 Volume aumentado para {self.radio.getVolume()}")
                
            elif payload.lower() in ['down', '-']:
                self.radio.setVolumeDown() 
                print(f"🔉 Volume diminuído para {self.radio.getVolume()}")
                
            else:
                volume = int(payload)
                if 0 <= volume <= 15:
                    self.radio.setVolume(volume)
                    print(f"🎵 Volume definido para {self.radio.getVolume()}")
                else:
                    print(f"❌ Volume deve ser entre 0 e 15, recebido: {volume}")
                    
        except ValueError:
            print(f"❌ Valor de volume inválido: {payload}")
    
    def handle_general_command(self, payload):
        """Processa comandos gerais"""
        try:
            # Tentar interpretar como JSON primeiro
            try:
                cmd_data = json.loads(payload)
                command = cmd_data.get('cmd', '').lower()
                params = cmd_data.get('params', {})
            except json.JSONDecodeError:
                # Se não for JSON, tratar como comando simples
                command = payload.lower()
                params = {}
            
            if command in ['mute', 'mute_on']:
                self.radio.setMute(True)
                print("🔇 Mudo ligado")
                
            elif command in ['unmute', 'mute_off']:
                self.radio.setMute(False)
                print("🔊 Mudo desligado")
                
            elif command == 'mute_toggle':
                self.radio.setMute(not self.radio.isMuted())
                print(f"🔇 Mudo {'ligado' if self.radio.isMuted() else 'desligado'}")
                
            elif command in ['bass_on', 'bass']:
                self.radio.setBass(True)
                print("🎵 Bass boost ligado")
                
            elif command == 'bass_off':
                self.radio.setBass(False)
                print("🎵 Bass boost desligado")
                
            elif command == 'bass_toggle':
                self.radio.setBass(not self.radio.getBass())
                print(f"🎵 Bass boost {'ligado' if self.radio.getBass() else 'desligado'}")
                
            elif command in ['mono', 'force_mono']:
                self.radio.setMono(True)
                print("📻 Modo mono ativado")
                
            elif command in ['stereo', 'allow_stereo']:
                self.radio.setMono(False)
                print("📻 Modo estéreo ativado")
                
            elif command == 'mono_toggle':
                self.radio.setMono(self.radio.isStereo())
                print(f"📻 Modo {'mono' if not self.radio.isStereo() else 'estéreo'}")
                
            elif command in ['rds_on', 'rds']:
                self.radio.setRDS(True)
                print("📡 RDS ligado")
                
            elif command == 'rds_off':
                self.radio.setRDS(False)
                print("📡 RDS desligado")
                
            elif command == 'seek_up':
                print("🔍 Buscando estação para cima...")
                self.radio.seek(rda5807.RDA_SEEK_WRAP, rda5807.RDA_SEEK_UP)
                print(f"📻 Estação encontrada: {self.radio.getFrequency()/100:.1f} MHz")
                
            elif command == 'seek_down':
                print("🔍 Buscando estação para baixo...")
                self.radio.seek(rda5807.RDA_SEEK_WRAP, rda5807.RDA_SEEK_DOWN)
                print(f"📻 Estação encontrada: {self.radio.getFrequency()/100:.1f} MHz")
                
            elif command == 'status':
                self.publish_status()
                print("📊 Status publicado")
                
            elif command == 'reset':
                self.radio.reset()
                time.sleep(0.5)
                self.radio.setup()
                print("🔄 Rádio reinicializado")
                
            else:
                print(f"❌ Comando desconhecido: {command}")
                
        except Exception as e:
            print(f"❌ Erro ao processar comando: {e}")
    
    def publish_status(self):
        """Publica status atual do rádio"""
        if not self.connected or not self.client:
            return
        
        try:
            status = self.radio.getStatus()
            status_json = json.dumps(status, indent=2)
            
            self.client.publish(self.topic_status, status_json, qos=0, retain=True)
            print(f"📤 Status publicado no tópico {self.topic_status}")
            
        except Exception as e:
            print(f"❌ Erro ao publicar status: {e}")
    
    def start(self):
        """Inicia o cliente MQTT"""
        if not MQTT_AVAILABLE:
            print("❌ Biblioteca MQTT não disponível")
            return False
        
        print(f"🚀 Iniciando cliente MQTT RDA5807...")
        print(f"📡 Broker: {self.broker_host}:{self.broker_port}")
        print(f"🎵 Rádio: {'Simulação' if self.simulate else 'Hardware real'}")
        
        # Status inicial do rádio
        print("\n📻 Status inicial do rádio:")
        self.radio.printStatus()
        
        try:
            # Conectar ao broker
            self.client.connect(self.broker_host, self.broker_port, 60)
            
            # Iniciar loop MQTT
            self.running = True
            self.client.loop_start()
            
            print(f"\n📋 Tópicos MQTT disponíveis:")
            print(f"   • {self.topic_frequency} - Controle de frequência")
            print(f"   • {self.topic_volume} - Controle de volume") 
            print(f"   • {self.topic_command} - Comandos gerais")
            print(f"   • {self.topic_status} - Status do rádio")
            
            print(f"\n💡 Exemplos de comandos:")
            print(f"   mosquitto_pub -h {self.broker_host} -t {self.topic_frequency} -m '103.9'")
            print(f"   mosquitto_pub -h {self.broker_host} -t {self.topic_volume} -m '10'")
            print(f"   mosquitto_pub -h {self.broker_host} -t {self.topic_command} -m 'seek_up'")
            
            return True
            
        except Exception as e:
            print(f"❌ Erro ao iniciar cliente MQTT: {e}")
            return False
    
    def stop(self):
        """Para o cliente MQTT"""
        print("\n🛑 Parando cliente MQTT...")
        self.running = False
        
        if self.client and self.connected:
            # Publicar status offline
            offline_status = {"status": "offline", "timestamp": time.time()}
            self.client.publish(self.topic_status, json.dumps(offline_status), retain=True)
            
            self.client.loop_stop()
            self.client.disconnect()
        
        print("✅ Cliente MQTT parado")
    
    def run_forever(self):
        """Executa o cliente indefinidamente"""
        if not self.start():
            return
        
        try:
            print("\n⏳ Cliente MQTT executando... (Pressione Ctrl+C para parar)")
            
            # Loop principal com status periódico
            while self.running:
                time.sleep(30)  # Publicar status a cada 30 segundos
                if self.connected:
                    self.publish_status()
                    
        except KeyboardInterrupt:
            print("\n\n⚠️  Interrupção detectada...")
        finally:
            self.stop()


def main():
    """Função principal"""
    print("Cliente MQTT RDA5807")
    print("===================")
    
    # Configurações padrão
    broker_host = "localhost"
    broker_port = 1883
    simulate = True
    
    # Verificar argumentos da linha de comando
    if len(sys.argv) > 1:
        broker_host = sys.argv[1]
    if len(sys.argv) > 2:
        broker_port = int(sys.argv[2])
    if len(sys.argv) > 3:
        simulate = sys.argv[3].lower() not in ['false', '0', 'no']
    
    print(f"Broker: {broker_host}:{broker_port}")
    print(f"Modo: {'Simulação' if simulate else 'Hardware'}")
    
    # Criar e executar cliente
    client = RDA5807MQTTClient(broker_host, broker_port, simulate)
    client.run_forever()

if __name__ == "__main__":
    main()