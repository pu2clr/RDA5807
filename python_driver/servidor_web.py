#!/usr/bin/env python3
"""
Servidor HTTP para controlar o RDA5807 via web
Demonstra integração do driver com interface web
"""

import sys
import os
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
import threading
import time

# Adicionar o diretório do driver ao path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import rda5807

class RDA5807Handler(BaseHTTPRequestHandler):
    """Handler para requisições HTTP do controle do rádio"""
    
    def __init__(self, radio_instance, *args, **kwargs):
        self.radio = radio_instance
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        """Processa requisições GET"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == '/':
            self.serve_html()
        elif path == '/status':
            self.serve_status()
        elif path == '/api/status':
            self.serve_api_status()
        else:
            self.send_error(404, "Página não encontrada")
    
    def do_POST(self):
        """Processa requisições POST (controles do rádio)"""
        parsed_path = urlparse(self.path)
        path = parsed_path.path
        
        if path == '/api/control':
            self.handle_control()
        else:
            self.send_error(404, "Endpoint não encontrado")
    
    def serve_html(self):
        """Serve a página HTML de controle"""
        html = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle RDA5807</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            max-width: 800px; 
            margin: 0 auto; 
            padding: 20px;
            background: linear-gradient(135deg, #1e3c72, #2a5298);
            color: white;
            min-height: 100vh;
        }
        .container {
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 30px;
            backdrop-filter: blur(10px);
        }
        h1 { 
            text-align: center; 
            color: #fff;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .status-card {
            background: rgba(255,255,255,0.2);
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            border: 1px solid rgba(255,255,255,0.3);
        }
        .control-group {
            margin: 20px 0;
            padding: 15px;
            border: 1px solid rgba(255,255,255,0.3);
            border-radius: 8px;
            background: rgba(255,255,255,0.1);
        }
        .control-group h3 {
            margin-top: 0;
            color: #ffd700;
        }
        button {
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
            color: white;
            border: none;
            padding: 10px 20px;
            margin: 5px;
            border-radius: 5px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.3s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.3);
        }
        button:active {
            transform: translateY(0);
        }
        input[type="number"], input[type="text"] {
            padding: 8px;
            border-radius: 4px;
            border: 1px solid #ccc;
            margin: 5px;
        }
        .frequency-display {
            font-size: 2em;
            font-weight: bold;
            text-align: center;
            background: rgba(0,0,0,0.3);
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }
        .volume-display {
            font-size: 1.5em;
            text-align: center;
            padding: 10px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            margin: 10px 0;
            padding: 5px 0;
            border-bottom: 1px solid rgba(255,255,255,0.2);
        }
        .indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-left: 10px;
        }
        .indicator.on { background-color: #4CAF50; }
        .indicator.off { background-color: #f44336; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎵 Controle RDA5807 🎵</h1>
        
        <div class="status-card">
            <div class="frequency-display" id="frequency-display">
                103.9 MHz
            </div>
            <div class="volume-display" id="volume-display">
                Volume: 8/15
            </div>
        </div>

        <div class="grid">
            <div class="control-group">
                <h3>📻 Frequência</h3>
                <button onclick="frequencyUp()">▲ Subir</button>
                <button onclick="frequencyDown()">▼ Descer</button><br>
                <input type="number" id="freq-input" placeholder="103.9" step="0.1" min="87.5" max="108.0">
                <button onclick="setFrequency()">Sintonizar</button><br>
                <button onclick="seekUp()">🔍 Buscar ▲</button>
                <button onclick="seekDown()">🔍 Buscar ▼</button>
            </div>

            <div class="control-group">
                <h3>🔊 Volume</h3>
                <button onclick="volumeUp()">+ Aumentar</button>
                <button onclick="volumeDown()">- Diminuir</button><br>
                <input type="number" id="vol-input" placeholder="8" min="0" max="15">
                <button onclick="setVolume()">Definir</button><br>
                <button onclick="toggleMute()">🔇 Mudo</button>
            </div>

            <div class="control-group">
                <h3>⚙️ Configurações</h3>
                <button onclick="toggleBass()">🎵 Bass Boost</button>
                <button onclick="toggleMono()">📻 Mono/Estéreo</button>
                <button onclick="toggleRds()">📡 RDS</button><br>
                <select id="band-select">
                    <option value="0">US/EU (87.5-108 MHz)</option>
                    <option value="1">Japan (76-91 MHz)</option>
                    <option value="2">World (76-108 MHz)</option>
                    <option value="3">Special (65-76 MHz)</option>
                </select>
                <button onclick="setBand()">Mudar Banda</button>
            </div>
        </div>

        <div class="status-card">
            <h3>📊 Status do Sistema</h3>
            <div class="status-item">
                <span>Mudo:</span>
                <span id="mute-status">Não <span class="indicator off"></span></span>
            </div>
            <div class="status-item">
                <span>Estéreo:</span>
                <span id="stereo-status">Sim <span class="indicator on"></span></span>
            </div>
            <div class="status-item">
                <span>Bass Boost:</span>
                <span id="bass-status">Não <span class="indicator off"></span></span>
            </div>
            <div class="status-item">
                <span>RDS:</span>
                <span id="rds-status">Não <span class="indicator off"></span></span>
            </div>
            <div class="status-item">
                <span>RSSI:</span>
                <span id="rssi-status">45</span>
            </div>
            <div class="status-item">
                <span>Banda:</span>
                <span id="band-status">US/EU</span>
            </div>
        </div>
    </div>

    <script>
        // Atualizar status a cada 2 segundos
        setInterval(updateStatus, 2000);
        
        // Carregar status inicial
        updateStatus();

        async function sendCommand(command, params = {}) {
            try {
                const response = await fetch('/api/control', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({command, params})
                });
                const result = await response.json();
                if (result.success) {
                    updateStatus();
                } else {
                    alert('Erro: ' + result.error);
                }
            } catch (error) {
                alert('Erro de comunicação: ' + error.message);
            }
        }

        async function updateStatus() {
            try {
                const response = await fetch('/api/status');
                const status = await response.json();
                
                document.getElementById('frequency-display').textContent = status.frequency_mhz.toFixed(1) + ' MHz';
                document.getElementById('volume-display').textContent = `Volume: ${status.volume}/15`;
                
                document.getElementById('mute-status').innerHTML = 
                    status.is_muted ? 'Sim <span class="indicator on"></span>' : 'Não <span class="indicator off"></span>';
                document.getElementById('stereo-status').innerHTML = 
                    status.is_stereo ? 'Sim <span class="indicator on"></span>' : 'Não <span class="indicator off"></span>';
                document.getElementById('bass-status').innerHTML = 
                    status.bass_enabled ? 'Sim <span class="indicator on"></span>' : 'Não <span class="indicator off"></span>';
                document.getElementById('rds-status').innerHTML = 
                    status.rds_enabled ? 'Sim <span class="indicator on"></span>' : 'Não <span class="indicator off"></span>';
                document.getElementById('rssi-status').textContent = status.rssi;
                
                const bandNames = ['US/EU', 'Japan', 'World', 'Special'];
                document.getElementById('band-status').textContent = bandNames[status.band];
                
            } catch (error) {
                console.error('Erro ao atualizar status:', error);
            }
        }

        // Funções de controle
        function frequencyUp() { sendCommand('frequency_up'); }
        function frequencyDown() { sendCommand('frequency_down'); }
        function volumeUp() { sendCommand('volume_up'); }
        function volumeDown() { sendCommand('volume_down'); }
        function toggleMute() { sendCommand('toggle_mute'); }
        function toggleBass() { sendCommand('toggle_bass'); }
        function toggleMono() { sendCommand('toggle_mono'); }
        function toggleRds() { sendCommand('toggle_rds'); }
        function seekUp() { sendCommand('seek', {direction: 'up'}); }
        function seekDown() { sendCommand('seek', {direction: 'down'}); }

        function setFrequency() {
            const freq = parseFloat(document.getElementById('freq-input').value);
            if (freq && freq >= 50 && freq <= 108) {
                sendCommand('set_frequency', {frequency: freq});
            } else {
                alert('Frequência inválida. Use valores entre 50.0 e 108.0 MHz');
            }
        }

        function setVolume() {
            const vol = parseInt(document.getElementById('vol-input').value);
            if (vol >= 0 && vol <= 15) {
                sendCommand('set_volume', {volume: vol});
            } else {
                alert('Volume inválido. Use valores entre 0 e 15');
            }
        }

        function setBand() {
            const band = parseInt(document.getElementById('band-select').value);
            sendCommand('set_band', {band: band});
        }
    </script>
</body>
</html>
        """
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html; charset=utf-8')
        self.end_headers()
        self.wfile.write(html.encode('utf-8'))
    
    def serve_api_status(self):
        """Retorna status em formato JSON"""
        try:
            status = self.radio.getStatus()
            response = json.dumps(status)
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(response.encode('utf-8'))
            
        except Exception as e:
            self.send_error(500, f"Erro interno: {str(e)}")
    
    def handle_control(self):
        """Processa comandos de controle do rádio"""
        try:
            # Ler dados da requisição
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            command = data.get('command')
            params = data.get('params', {})
            
            response = {'success': True, 'message': ''}
            
            # Processar comandos
            if command == 'frequency_up':
                self.radio.setFrequencyUp()
                response['message'] = f'Frequência: {self.radio.getFrequency()/100:.1f} MHz'
                
            elif command == 'frequency_down':
                self.radio.setFrequencyDown()
                response['message'] = f'Frequência: {self.radio.getFrequency()/100:.1f} MHz'
                
            elif command == 'set_frequency':
                freq = int(params['frequency'] * 100)  # Converter MHz para dezenas de kHz
                self.radio.setFrequency(freq)
                response['message'] = f'Frequência: {self.radio.getFrequency()/100:.1f} MHz'
                
            elif command == 'volume_up':
                self.radio.setVolumeUp()
                response['message'] = f'Volume: {self.radio.getVolume()}'
                
            elif command == 'volume_down':
                self.radio.setVolumeDown()
                response['message'] = f'Volume: {self.radio.getVolume()}'
                
            elif command == 'set_volume':
                self.radio.setVolume(params['volume'])
                response['message'] = f'Volume: {self.radio.getVolume()}'
                
            elif command == 'toggle_mute':
                self.radio.setMute(not self.radio.isMuted())
                response['message'] = f'Mudo: {"Ligado" if self.radio.isMuted() else "Desligado"}'
                
            elif command == 'toggle_bass':
                self.radio.setBass(not self.radio.getBass())
                response['message'] = f'Bass: {"Ligado" if self.radio.getBass() else "Desligado"}'
                
            elif command == 'toggle_mono':
                self.radio.setMono(self.radio.isStereo())  # Inverter
                response['message'] = f'Áudio: {"Mono" if not self.radio.isStereo() else "Estéreo"}'
                
            elif command == 'toggle_rds':
                new_rds = not self.radio.rds_enabled
                self.radio.setRDS(new_rds)
                response['message'] = f'RDS: {"Ligado" if new_rds else "Desligado"}'
                
            elif command == 'seek':
                direction = rda5807.RDA_SEEK_UP if params.get('direction') == 'up' else rda5807.RDA_SEEK_DOWN
                self.radio.seek(rda5807.RDA_SEEK_WRAP, direction)
                response['message'] = f'Busca concluída: {self.radio.getFrequency()/100:.1f} MHz'
                
            elif command == 'set_band':
                self.radio.setBand(params['band'])
                band_names = ['US/EU', 'Japan', 'World', 'Special']
                response['message'] = f'Banda: {band_names[params["band"]]}'
                
            else:
                response = {'success': False, 'error': f'Comando desconhecido: {command}'}
            
            # Enviar resposta
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except Exception as e:
            error_response = {'success': False, 'error': str(e)}
            self.send_response(500)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(error_response).encode('utf-8'))
    
    def log_message(self, format, *args):
        """Suprime logs HTTP padrão para saída mais limpa"""
        pass


def create_handler(radio_instance):
    """Cria handler com instância do rádio"""
    def handler(*args, **kwargs):
        RDA5807Handler(radio_instance, *args, **kwargs)
    return handler


def main():
    """Função principal do servidor web"""
    print("Servidor Web RDA5807")
    print("===================")
    
    # Inicializar rádio
    print("Inicializando rádio...")
    radio = rda5807.RDA5807(simulate=True)  # Modo simulação
    radio.setup()
    radio.setFrequency(10390)  # 103.9 MHz
    radio.setVolume(8)
    
    print("Rádio inicializado:")
    radio.printStatus()
    
    # Configurar servidor
    HOST = 'localhost'
    PORT = 8080
    
    handler = create_handler(radio)
    server = HTTPServer((HOST, PORT), handler)
    
    print(f"\nServidor iniciado em http://{HOST}:{PORT}")
    print("Pressione Ctrl+C para parar o servidor")
    print("\nAcesse http://localhost:8080 no seu navegador para controlar o rádio")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n\nParando servidor...")
        server.shutdown()
        print("Servidor parado.")

if __name__ == "__main__":
    main()