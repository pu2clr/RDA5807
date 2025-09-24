#!/usr/bin/env python3
"""
Exemplo de uso do driver RDA5807 Python
Demonstra como usar todas as principais funcionalidades
"""

import time
import sys
import os

# Adicionar o diretório do driver ao path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import rda5807

def exemplo_basico():
    """Exemplo básico de uso do driver"""
    print("=== EXEMPLO BÁSICO ===")
    
    # Criar instância do rádio (modo simulação)
    rx = rda5807.RDA5807(simulate=True)
    
    # Inicializar com configurações padrão
    rx.setup()
    
    # Configurar frequência e volume
    rx.setFrequency(10390)  # 103.9 MHz
    rx.setVolume(8)
    
    # Mostrar status inicial
    rx.printStatus()
    
    return rx

def exemplo_controles_frequencia(rx):
    """Demonstra controles de frequência"""
    print("\n=== CONTROLES DE FREQUÊNCIA ===")
    
    print(f"Frequência inicial: {rx.getFrequency()/100:.1f} MHz")
    
    # Incrementar frequência
    rx.setFrequencyUp()
    rx.setFrequencyUp() 
    rx.setFrequencyUp()
    print(f"Após 3 incrementos: {rx.getFrequency()/100:.1f} MHz")
    
    # Decrementar frequência  
    rx.setFrequencyDown()
    print(f"Após 1 decremento: {rx.getFrequency()/100:.1f} MHz")
    
    # Definir frequência específica
    rx.setFrequency(9590)  # 95.9 MHz
    print(f"Frequência definida para: {rx.getFrequency()/100:.1f} MHz")
    
    # Ir para início e fim da banda
    rx.setFrequencyToBeginBand()
    print(f"Início da banda: {rx.getFrequency()/100:.1f} MHz")
    
    rx.setFrequencyToEndBand()
    print(f"Fim da banda: {rx.getFrequency()/100:.1f} MHz")
    
    # Mostrar limites da banda
    print(f"Banda atual: {rx.getBand()}")
    print(f"Frequência mínima: {rx.getMinimumFrequencyOfTheBand()/100:.1f} MHz")
    print(f"Frequência máxima: {rx.getMaximumFrequencyOfTheBand()/100:.1f} MHz")

def exemplo_controles_audio(rx):
    """Demonstra controles de áudio"""
    print("\n=== CONTROLES DE ÁUDIO ===")
    
    print(f"Volume inicial: {rx.getVolume()}")
    
    # Controlar volume
    rx.setVolumeUp()
    rx.setVolumeUp()
    print(f"Após 2 incrementos: {rx.getVolume()}")
    
    rx.setVolumeDown()
    print(f"Após 1 decremento: {rx.getVolume()}")
    
    # Definir volume específico
    rx.setVolume(12)
    print(f"Volume definido para: {rx.getVolume()}")
    
    # Testar mudo
    print(f"Está no mudo? {rx.isMuted()}")
    rx.setMute(True)
    print(f"Mudo ligado: {rx.isMuted()}")
    rx.setMute(False) 
    print(f"Mudo desligado: {rx.isMuted()}")
    
    # Testar bass boost
    print(f"Bass boost ativo? {rx.getBass()}")
    rx.setBass(True)
    print(f"Bass boost ligado: {rx.getBass()}")
    rx.setBass(False)
    print(f"Bass boost desligado: {rx.getBass()}")
    
    # Testar mono/estéreo
    print(f"Em estéreo? {rx.isStereo()}")
    rx.setMono(True)  # Força mono
    print(f"Forçando mono: {rx.isStereo()}")
    rx.setMono(False)  # Permite estéreo
    print(f"Permitindo estéreo: {rx.isStereo()}")

def exemplo_bandas(rx):
    """Demonstra mudança de bandas"""
    print("\n=== CONTROLES DE BANDA ===")
    
    bandas = {
        rda5807.RDA_FM_BAND_USA_EU: "US/Europe (87.5-108 MHz)",
        rda5807.RDA_FM_BAND_JAPAN_WIDE: "Japan (76-91 MHz)",
        rda5807.RDA_FM_BAND_WORLD: "World (76-108 MHz)",
        rda5807.RDA_FM_BAND_SPECIAL: "Special (65-76 MHz)"
    }
    
    for banda, nome in bandas.items():
        rx.setBand(banda)
        print(f"Banda {banda}: {nome}")
        print(f"  Faixa: {rx.getMinimumFrequencyOfTheBand()/100:.1f} - {rx.getMaximumFrequencyOfTheBand()/100:.1f} MHz")
        
        # Definir frequência no meio da banda
        min_freq = rx.getMinimumFrequencyOfTheBand()
        max_freq = rx.getMaximumFrequencyOfTheBand()
        freq_media = (min_freq + max_freq) // 2
        rx.setFrequency(freq_media)
        print(f"  Frequência teste: {rx.getFrequency()/100:.1f} MHz")
        print()

def exemplo_seek(rx):
    """Demonstra busca de estações"""
    print("\n=== BUSCA DE ESTAÇÕES ===")
    
    # Definir frequência inicial
    rx.setFrequency(10000)  # 100.0 MHz
    print(f"Frequência inicial: {rx.getFrequency()/100:.1f} MHz")
    
    # Função callback para mostrar progresso
    def mostrar_progresso():
        print(f"  Buscando... {rx.getFrequency()/100:.1f} MHz")
    
    # Buscar para cima
    print("Buscando estação para cima...")
    rx.seek(rda5807.RDA_SEEK_WRAP, rda5807.RDA_SEEK_UP, mostrar_progresso)
    print(f"Estação encontrada: {rx.getFrequency()/100:.1f} MHz")
    
    time.sleep(1)
    
    # Buscar para baixo
    print("\nBuscando estação para baixo...")
    rx.seek(rda5807.RDA_SEEK_WRAP, rda5807.RDA_SEEK_DOWN, mostrar_progresso)
    print(f"Estação encontrada: {rx.getFrequency()/100:.1f} MHz")

def exemplo_rds(rx):
    """Demonstra funcionalidades RDS"""
    print("\n=== RDS (Radio Data System) ===")
    
    # Ligar RDS
    rx.setRDS(True)
    rx.setRdsFifo(True)
    print("RDS habilitado")
    
    # Verificar se RDS está pronto
    print(f"RDS pronto? {rx.getRdsReady()}")
    
    if rx.getRdsReady():
        # Obter informações RDS (simuladas)
        station_name = rx.getRdsStationName()
        program_info = rx.getRdsProgramInformation()
        
        print(f"Nome da estação: {station_name}")
        print(f"Informação do programa: {program_info}")
    else:
        print("Dados RDS não disponíveis (modo simulação)")

def exemplo_status_completo(rx):
    """Mostra status completo do rádio"""
    print("\n=== STATUS COMPLETO ===")
    
    # Configurar alguns parâmetros
    rx.setFrequency(10650)  # 106.5 MHz
    rx.setVolume(10)
    rx.setBass(True)
    rx.setRDS(True)
    
    # Mostrar status formatado
    rx.printStatus()
    
    # Mostrar status como dicionário
    print("\nStatus como dicionário:")
    status = rx.getStatus()
    for key, value in status.items():
        print(f"  {key}: {value}")
    
    print(f"\nRSSI (força do sinal): {rx.getRssi()}")
    print(f"Device ID: {hex(rx.getDeviceId())}")

def exemplo_radio_interativo():
    """Exemplo de rádio interativo simples"""
    print("\n=== RÁDIO INTERATIVO ===")
    print("Comandos:")
    print("  f <freq> - Definir frequência (ex: f 103.9)")
    print("  v <vol>  - Definir volume (ex: v 8)")
    print("  u        - Frequência para cima")
    print("  d        - Frequência para baixo") 
    print("  +        - Volume para cima")
    print("  -        - Volume para baixo")
    print("  m        - Toggle mudo")
    print("  s        - Buscar estação para cima")
    print("  b        - Toggle bass boost")
    print("  i        - Mostrar status")
    print("  q        - Sair")
    print()
    
    rx = rda5807.RDA5807(simulate=True)
    rx.setup()
    rx.setFrequency(10390)
    rx.setVolume(8)
    
    rx.printStatus()
    
    while True:
        try:
            cmd = input("\nComando: ").strip().lower()
            
            if cmd == 'q':
                break
            elif cmd == 'i':
                rx.printStatus()
            elif cmd.startswith('f '):
                freq = float(cmd.split()[1]) * 100  # Converter MHz para dezenas de kHz
                rx.setFrequency(int(freq))
                print(f"Frequência: {rx.getFrequency()/100:.1f} MHz")
            elif cmd.startswith('v '):
                vol = int(cmd.split()[1])
                rx.setVolume(vol)
                print(f"Volume: {rx.getVolume()}")
            elif cmd == 'u':
                rx.setFrequencyUp()
                print(f"Frequência: {rx.getFrequency()/100:.1f} MHz")
            elif cmd == 'd':
                rx.setFrequencyDown() 
                print(f"Frequência: {rx.getFrequency()/100:.1f} MHz")
            elif cmd == '+':
                rx.setVolumeUp()
                print(f"Volume: {rx.getVolume()}")
            elif cmd == '-':
                rx.setVolumeDown()
                print(f"Volume: {rx.getVolume()}")
            elif cmd == 'm':
                rx.setMute(not rx.isMuted())
                print(f"Mudo: {'Ligado' if rx.isMuted() else 'Desligado'}")
            elif cmd == 's':
                print("Buscando estação...")
                rx.seek(rda5807.RDA_SEEK_WRAP, rda5807.RDA_SEEK_UP)
                print(f"Estação encontrada: {rx.getFrequency()/100:.1f} MHz")
            elif cmd == 'b':
                rx.setBass(not rx.getBass())
                print(f"Bass boost: {'Ligado' if rx.getBass() else 'Desligado'}")
            else:
                print("Comando inválido. Digite 'q' para sair.")
                
        except (ValueError, IndexError):
            print("Erro no comando. Verifique a sintaxe.")
        except KeyboardInterrupt:
            break
    
    print("\nSaindo do rádio interativo...")

def main():
    """Função principal - executa todos os exemplos"""
    print("Driver RDA5807 Python - Exemplos de Uso")
    print("=======================================")
    
    try:
        # Exemplo básico
        rx = exemplo_basico()
        
        # Exemplos específicos
        exemplo_controles_frequencia(rx)
        exemplo_controles_audio(rx)  
        exemplo_bandas(rx)
        exemplo_seek(rx)
        exemplo_rds(rx)
        exemplo_status_completo(rx)
        
        print("\n" + "="*50)
        print("Exemplos básicos concluídos!")
        print("Deseja executar o rádio interativo? (s/n): ", end="")
        
        resposta = input().strip().lower()
        if resposta in ['s', 'sim', 'y', 'yes']:
            exemplo_radio_interativo()
        
    except KeyboardInterrupt:
        print("\n\nInterrompido pelo usuário.")
    except Exception as e:
        print(f"\nErro durante execução: {e}")
        import traceback
        traceback.print_exc()
    
    print("\nTodos os exemplos foram executados!")

if __name__ == "__main__":
    main()