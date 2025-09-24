#!/usr/bin/env python3
"""
RDA5807 Python Driver
Biblioteca Python que replica a interface da biblioteca RDA5807 Arduino/C++
Autor: Baseado na biblioteca C++ de Ricardo Lima Caratti (pu2clr)
Data: 2025

Exemplo de uso:
    import rda5807
    
    # Criar instância do rádio
    rx = rda5807.RDA5807()
    
    # Configurar e inicializar
    rx.setup()
    rx.setFrequency(10390)  # 103.9 MHz
    rx.setVolume(8)
    
    # Controlar o rádio
    rx.setFrequencyUp()
    rx.setVolumeDown()
    rx.setMute(True)
"""

import time
import logging

try:
    import smbus
    I2C_AVAILABLE = True
except ImportError:
    I2C_AVAILABLE = False
    logging.warning("SMBus não disponível. Executando em modo simulação.")

# Constantes - Endereços I2C
I2C_ADDR_DIRECT_ACCESS = 0x11
I2C_ADDR_FULL_ACCESS = 0x10

# Constantes - Tipos de Oscilador
OSCILLATOR_TYPE_CRYSTAL = 0
OSCILLATOR_TYPE_PASSIVE = 0
OSCILLATOR_TYPE_REFCLK = 1
OSCILLATOR_TYPE_ACTIVE = 1

# Constantes - Frequências de Clock
CLOCK_32K = 0b000
CLOCK_12M = 0b001
CLOCK_13M = 0b010
CLOCK_19_2M = 0b011
CLOCK_24M = 0b101
CLOCK_26M = 0b110
CLOCK_38_4M = 0b111

# Constantes - Bandas FM
RDA_FM_BAND_USA_EU = 0      # 87.5–108 MHz (US / Europe, Default)
RDA_FM_BAND_JAPAN_WIDE = 1  # 76–91 MHz (Japan wide band)  
RDA_FM_BAND_WORLD = 2       # 76–108 MHz (world wide)
RDA_FM_BAND_SPECIAL = 3     # 65–76 MHz(East Europe) or 50-65MHz

# Constantes - Seek
RDA_SEEK_WRAP = 0  # Wrap at the upper or lower band limit and continue seeking
RDA_SEEK_STOP = 1  # Stop seeking at the upper or lower band limit
RDA_SEEK_DOWN = 0  # Seek Down
RDA_SEEK_UP = 1    # Seek Up

# Registradores
REG00 = 0x00
REG02 = 0x02
REG03 = 0x03
REG04 = 0x04
REG05 = 0x05
REG06 = 0x06
REG07 = 0x07
REG08 = 0x08
REG0A = 0x0A
REG0B = 0x0B
REG0C = 0x0C
REG0D = 0x0D
REG0E = 0x0E
REG0F = 0x0F


class RDA5807:
    """
    Classe principal do driver RDA5807 Python
    Replica a interface da biblioteca C++ Arduino
    """
    
    def __init__(self, i2c_bus=1, simulate=False):
        """
        Inicializa o driver RDA5807
        
        Args:
            i2c_bus (int): Número do barramento I2C (padrão: 1)
            simulate (bool): Se True, executa em modo simulação sem hardware
        """
        self.simulate = simulate or not I2C_AVAILABLE
        self.i2c_bus_number = i2c_bus
        self.bus = None
        
        # Estado interno
        self.current_frequency = 10390  # 103.9 MHz padrão
        self.current_volume = 6
        self.current_band = RDA_FM_BAND_USA_EU
        self.is_muted = False
        self.is_stereo = True
        self.bass_enabled = False
        self.rds_enabled = False
        self.power_enabled = False
        
        # Limites de banda [min, max] em dezenas de kHz
        self.band_limits = {
            RDA_FM_BAND_USA_EU: [8750, 10800],      # 87.5-108 MHz
            RDA_FM_BAND_JAPAN_WIDE: [7600, 9100],   # 76-91 MHz
            RDA_FM_BAND_WORLD: [7600, 10800],       # 76-108 MHz
            RDA_FM_BAND_SPECIAL: [6500, 7600]       # 65-76 MHz
        }
        
        # Inicializar I2C se não estiver em modo simulação
        if not self.simulate:
            try:
                self.bus = smbus.SMBus(self.i2c_bus_number)
                logging.info(f"I2C inicializado no barramento {self.i2c_bus_number}")
            except Exception as e:
                logging.error(f"Erro ao inicializar I2C: {e}")
                self.simulate = True
        
        if self.simulate:
            logging.info("Executando em modo simulação")
    
    def _write_register(self, reg, value):
        """Escreve um valor em um registrador"""
        if self.simulate:
            logging.debug(f"[SIM] Escrevendo reg 0x{reg:02X} = 0x{value:04X}")
            return
        
        try:
            # Converter para bytes (big endian)
            high_byte = (value >> 8) & 0xFF
            low_byte = value & 0xFF
            self.bus.write_i2c_block_data(I2C_ADDR_FULL_ACCESS, reg, [high_byte, low_byte])
        except Exception as e:
            logging.error(f"Erro ao escrever registrador 0x{reg:02X}: {e}")
    
    def _read_register(self, reg):
        """Lê um valor de um registrador"""
        if self.simulate:
            # Retorna valores simulados
            if reg == REG0A:
                return 0x5800  # Status simulado
            elif reg == REG0B:
                return 0x0000  # RDS simulado
            return 0x0000
        
        try:
            data = self.bus.read_i2c_block_data(I2C_ADDR_DIRECT_ACCESS, reg, 2)
            return (data[0] << 8) | data[1]
        except Exception as e:
            logging.error(f"Erro ao ler registrador 0x{reg:02X}: {e}")
            return 0x0000

    # ============ FUNÇÕES DE CONFIGURAÇÃO BÁSICA ============
    
    def setup(self, clock_frequency=CLOCK_32K, oscillator_type=OSCILLATOR_TYPE_PASSIVE, rlck_no_calibrate=0):
        """
        Inicializa o rádio RDA5807 com parâmetros padrão
        
        Args:
            clock_frequency: Frequência do clock (padrão: CLOCK_32K)
            oscillator_type: Tipo de oscilador (padrão: OSCILLATOR_TYPE_PASSIVE)  
            rlck_no_calibrate: Modo de calibração (padrão: 0)
        """
        logging.info("Inicializando RDA5807...")
        
        # Reset e configuração inicial
        self._write_register(REG02, 0x0002)  # Soft reset
        time.sleep(0.1)
        
        # Configuração básica - Enable, Clock, Bass, Stereo
        reg02_value = 0x0001  # ENABLE = 1
        reg02_value |= (clock_frequency << 4)  # CLK_MODE
        reg02_value |= (0 << 8)  # SEEK = 0
        reg02_value |= (0 << 9)  # SEEKUP = 0
        reg02_value |= (0 << 11) # NON_CALIBRATE
        reg02_value |= (0 << 12) # BASS = 0
        reg02_value |= (0 << 13) # MONO = 0 (stereo)
        reg02_value |= (1 << 14) # DMUTE = 1 (unmute)
        reg02_value |= (1 << 15) # DHIZ = 1 (normal operation)
        
        self._write_register(REG02, reg02_value)
        
        # Configurar banda padrão
        self.setBand(self.current_band)
        
        # Configurar volume padrão
        self.setVolume(self.current_volume)
        
        self.power_enabled = True
        logging.info("RDA5807 inicializado com sucesso")
    
    def reset(self):
        """Executa reset do chip"""
        logging.info("Executando reset do RDA5807")
        self._write_register(REG02, 0x0002)  # Soft reset
        time.sleep(0.1)
        self.power_enabled = False

    # ============ FUNÇÕES DE CONTROLE DE FREQUÊNCIA ============
    
    def setFrequency(self, frequency):
        """
        Define a frequência de sintonia
        
        Args:
            frequency (int): Frequência em dezenas de kHz (ex: 10390 para 103.9 MHz)
        """
        # Validar limites da banda atual
        min_freq, max_freq = self.band_limits[self.current_band]
        frequency = max(min_freq, min(max_freq, frequency))
        
        self.current_frequency = frequency
        
        # Calcular valor do canal
        if self.current_band == RDA_FM_BAND_USA_EU:
            channel = frequency - 8750  # Base 87.5 MHz
        elif self.current_band == RDA_FM_BAND_JAPAN_WIDE:
            channel = frequency - 7600  # Base 76 MHz  
        elif self.current_band == RDA_FM_BAND_WORLD:
            channel = frequency - 7600  # Base 76 MHz
        else:  # RDA_FM_BAND_SPECIAL
            channel = frequency - 6500  # Base 65 MHz
        
        # Configurar registrador 0x03
        reg03_value = 0x0000
        reg03_value |= (channel << 6)  # CHAN
        reg03_value |= (1 << 4)       # TUNE = 1
        reg03_value |= (self.current_band << 2)  # BAND
        reg03_value |= 0x00           # SPACE = 100kHz
        
        self._write_register(REG03, reg03_value)
        
        # Aguardar tune completar
        time.sleep(0.1)
        
        logging.info(f"Frequência ajustada para {frequency/100:.1f} MHz")
    
    def getFrequency(self):
        """
        Obtém a frequência atual
        
        Returns:
            int: Frequência atual em dezenas de kHz
        """
        return self.current_frequency
    
    def getRealFrequency(self):
        """
        Obtém a frequência real do hardware
        
        Returns:
            int: Frequência real em dezenas de kHz  
        """
        if self.simulate:
            return self.current_frequency
            
        # Ler registrador 0x0A para obter a frequência atual
        reg0a = self._read_register(REG0A)
        channel = (reg0a >> 0) & 0x03FF  # Bits 9:0
        
        # Converter canal para frequência baseado na banda
        if self.current_band == RDA_FM_BAND_USA_EU:
            return channel + 8750
        elif self.current_band == RDA_FM_BAND_JAPAN_WIDE:
            return channel + 7600
        elif self.current_band == RDA_FM_BAND_WORLD:
            return channel + 7600
        else:  # RDA_FM_BAND_SPECIAL
            return channel + 6500
    
    def setFrequencyUp(self):
        """Incrementa a frequência em 100 kHz"""
        new_freq = self.current_frequency + 1  # +100 kHz
        self.setFrequency(new_freq)
    
    def setFrequencyDown(self):
        """Decrementa a frequência em 100 kHz"""  
        new_freq = self.current_frequency - 1  # -100 kHz
        self.setFrequency(new_freq)
    
    def setFrequencyToBeginBand(self):
        """Define a frequência para o início da banda atual"""
        min_freq, _ = self.band_limits[self.current_band]
        self.setFrequency(min_freq)
    
    def setFrequencyToEndBand(self):
        """Define a frequência para o fim da banda atual"""
        _, max_freq = self.band_limits[self.current_band]
        self.setFrequency(max_freq)
    
    def getMinimumFrequencyOfTheBand(self):
        """Retorna a frequência mínima da banda atual"""
        min_freq, _ = self.band_limits[self.current_band]
        return min_freq
    
    def getMaximumFrequencyOfTheBand(self):
        """Retorna a frequência máxima da banda atual"""
        _, max_freq = self.band_limits[self.current_band]
        return max_freq

    # ============ FUNÇÕES DE SEEK ============
    
    def seek(self, seek_mode=RDA_SEEK_WRAP, direction=RDA_SEEK_UP, callback=None):
        """
        Executa busca automática de estações
        
        Args:
            seek_mode: RDA_SEEK_WRAP ou RDA_SEEK_STOP
            direction: RDA_SEEK_UP ou RDA_SEEK_DOWN
            callback: Função opcional chamada durante a busca
        """
        logging.info(f"Iniciando busca {'UP' if direction else 'DOWN'}")
        
        # Configurar registrador 0x02 para seek
        reg02_value = 0x8000  # Base config
        reg02_value |= (1 << 8)  # SEEK = 1
        reg02_value |= (seek_mode << 7)  # SKMODE
        reg02_value |= (direction << 9)  # SEEKUP
        
        self._write_register(REG02, reg02_value)
        
        if self.simulate:
            # Simular busca
            if direction == RDA_SEEK_UP:
                self.current_frequency += 5  # +500 kHz
            else:
                self.current_frequency -= 5  # -500 kHz
            
            # Validar limites
            min_freq, max_freq = self.band_limits[self.current_band]
            if self.current_frequency > max_freq:
                if seek_mode == RDA_SEEK_WRAP:
                    self.current_frequency = min_freq
                else:
                    self.current_frequency = max_freq
            elif self.current_frequency < min_freq:
                if seek_mode == RDA_SEEK_WRAP:
                    self.current_frequency = max_freq
                else:
                    self.current_frequency = min_freq
            
            if callback:
                callback()
            
            time.sleep(1)  # Simular tempo de busca
        else:
            # Aguardar seek completar (STC bit)
            timeout = 0
            while timeout < 50:  # 5 segundos máximo
                reg0a = self._read_register(REG0A)
                if (reg0a >> 14) & 1:  # STC bit
                    break
                if callback:
                    callback()
                time.sleep(0.1)
                timeout += 1
            
            # Atualizar frequência atual
            self.current_frequency = self.getRealFrequency()
        
        logging.info(f"Busca completada. Nova frequência: {self.current_frequency/100:.1f} MHz")

    # ============ FUNÇÕES DE CONTROLE DE ÁUDIO ============
    
    def setVolume(self, volume):
        """
        Define o volume do áudio
        
        Args:
            volume (int): Volume de 0 a 15
        """
        volume = max(0, min(15, volume))  # Limitar entre 0-15
        self.current_volume = volume
        
        # Escrever no registrador 0x05
        reg05_value = volume  # Volume nos bits 3:0
        self._write_register(REG05, reg05_value)
        
        logging.info(f"Volume ajustado para {volume}")
    
    def getVolume(self):
        """
        Obtém o volume atual
        
        Returns:
            int: Volume atual (0-15)
        """
        return self.current_volume
    
    def setVolumeUp(self):
        """Incrementa o volume"""
        if self.current_volume < 15:
            self.setVolume(self.current_volume + 1)
    
    def setVolumeDown(self):
        """Decrementa o volume"""
        if self.current_volume > 0:
            self.setVolume(self.current_volume - 1)
    
    def setMute(self, mute=True):
        """
        Liga/desliga o mudo
        
        Args:
            mute (bool): True para ligar mudo, False para desligar
        """
        self.is_muted = mute
        
        # Atualizar registrador 0x02
        reg02_value = 0x8000  # Base config
        reg02_value |= (0 if mute else 1) << 14  # DMUTE (invertido)
        
        self._write_register(REG02, reg02_value)
        
        logging.info(f"Mudo {'ligado' if mute else 'desligado'}")
    
    def getMute(self):
        """Retorna True se estiver no mudo"""
        return self.is_muted
    
    def isMuted(self):
        """Retorna True se estiver no mudo"""
        return self.is_muted

    def setBass(self, enable=True):
        """
        Liga/desliga reforço de graves
        
        Args:
            enable (bool): True para ligar bass boost
        """
        self.bass_enabled = enable
        
        # Atualizar registrador 0x02
        reg02_value = 0x8000  # Base config
        reg02_value |= (1 if enable else 0) << 12  # BASS
        
        self._write_register(REG02, reg02_value)
        
        logging.info(f"Bass boost {'ligado' if enable else 'desligado'}")
    
    def getBass(self):
        """Retorna True se bass boost estiver ligado"""
        return self.bass_enabled

    def setMono(self, force_mono=True):
        """
        Força modo mono ou permite estéreo
        
        Args:
            force_mono (bool): True para forçar mono, False para permitir estéreo
        """
        self.is_stereo = not force_mono
        
        # Atualizar registrador 0x02  
        reg02_value = 0x8000  # Base config
        reg02_value |= (1 if force_mono else 0) << 13  # MONO
        
        self._write_register(REG02, reg02_value)
        
        logging.info(f"Modo {'mono' if force_mono else 'stereo'}")
    
    def isStereo(self):
        """Retorna True se estiver em estéreo"""
        if self.simulate:
            return self.is_stereo
        
        # Ler status do registrador 0x0A
        reg0a = self._read_register(REG0A)
        return bool((reg0a >> 10) & 1)  # ST bit

    # ============ FUNÇÕES DE BANDA ============
    
    def setBand(self, band=RDA_FM_BAND_USA_EU):
        """
        Define a banda FM
        
        Args:
            band (int): Banda FM (0=US/EU, 1=Japan, 2=World, 3=Special)
        """
        band = max(0, min(3, band))  # Validar
        self.current_band = band
        
        # Atualizar registrador 0x03
        reg03_value = 0x0000
        reg03_value |= (band << 2)  # BAND
        
        self._write_register(REG03, reg03_value)
        
        band_names = ["US/EU (87.5-108MHz)", "Japan (76-91MHz)", "World (76-108MHz)", "Special (65-76MHz)"]
        logging.info(f"Banda alterada para: {band_names[band]}")
    
    def getBand(self):
        """Retorna a banda atual"""
        return self.current_band

    # ============ FUNÇÕES RDS ============
    
    def setRDS(self, enable=True):
        """
        Liga/desliga RDS
        
        Args:
            enable (bool): True para ligar RDS
        """
        self.rds_enabled = enable
        
        # Atualizar registrador 0x02
        reg02_value = 0x8000  # Base config
        reg02_value |= (1 if enable else 0) << 3  # RDS_EN
        
        self._write_register(REG02, reg02_value)
        
        logging.info(f"RDS {'ligado' if enable else 'desligado'}")
    
    def setRdsFifo(self, enable=True):
        """Liga/desliga FIFO do RDS"""
        logging.info(f"RDS FIFO {'ligado' if enable else 'desligado'}")
    
    def getRdsReady(self):
        """Retorna True se dados RDS estão prontos"""
        if self.simulate:
            return self.rds_enabled
        
        reg0a = self._read_register(REG0A)
        return bool((reg0a >> 15) & 1)  # RDSR bit
    
    def getRdsStationName(self):
        """Retorna o nome da estação (simulado)"""
        if self.simulate:
            return "TEST FM"
        return "UNKNOWN"
    
    def getRdsProgramInformation(self):
        """Retorna informação do programa (simulado)"""
        if self.simulate:
            return "Python RDA5807 Driver Test"
        return ""

    # ============ FUNÇÕES DE STATUS ============
    
    def getRssi(self):
        """
        Obtém o valor RSSI (força do sinal)
        
        Returns:
            int: Valor RSSI (0-127, quanto maior melhor)
        """
        if self.simulate:
            return 45  # Valor simulado
        
        reg0b = self._read_register(REG0B)
        return (reg0b >> 9) & 0x7F  # Bits 15:9

    def getDeviceId(self):
        """Retorna o ID do dispositivo"""
        if self.simulate:
            return 0x5808  # ID simulado do RDA5807
        
        return self._read_register(REG00)

    # ============ FUNÇÕES DE UTILIDADE ============
    
    def getStatus(self):
        """
        Retorna dicionário com status completo do rádio
        
        Returns:
            dict: Status completo do rádio
        """
        return {
            'frequency': self.current_frequency,
            'frequency_mhz': self.current_frequency / 100.0,
            'volume': self.current_volume,
            'band': self.current_band,
            'is_muted': self.is_muted,
            'is_stereo': self.is_stereo,
            'bass_enabled': self.bass_enabled,
            'rds_enabled': self.rds_enabled,
            'power_enabled': self.power_enabled,
            'rssi': self.getRssi(),
            'device_id': hex(self.getDeviceId())
        }
    
    def printStatus(self):
        """Imprime o status atual do rádio"""
        status = self.getStatus()
        print("=== STATUS RDA5807 ===")
        print(f"Frequência: {status['frequency_mhz']:.1f} MHz")
        print(f"Volume: {status['volume']}/15")
        print(f"Banda: {status['band']}")
        print(f"Mudo: {'Sim' if status['is_muted'] else 'Não'}")
        print(f"Estéreo: {'Sim' if status['is_stereo'] else 'Não'}")
        print(f"Bass Boost: {'Sim' if status['bass_enabled'] else 'Não'}")
        print(f"RDS: {'Sim' if status['rds_enabled'] else 'Não'}")
        print(f"RSSI: {status['rssi']}")
        print(f"Device ID: {status['device_id']}")
        print("=====================")


def main():
    """Função principal para teste do driver"""
    print("Teste do Driver RDA5807 Python")
    print("==============================")
    
    # Criar instância do rádio
    rx = RDA5807(simulate=True)  # Modo simulação para teste
    
    # Inicializar
    rx.setup()
    
    # Configurar parâmetros
    rx.setFrequency(10390)  # 103.9 MHz
    rx.setVolume(8)
    rx.setRDS(True)
    
    # Mostrar status
    rx.printStatus()
    
    print("\nTestando controles...")
    
    # Testar controles
    rx.setVolumeUp()
    print(f"Volume após incremento: {rx.getVolume()}")
    
    rx.setFrequencyUp()
    print(f"Frequência após incremento: {rx.getFrequency()/100:.1f} MHz")
    
    rx.setMute(True)
    print(f"Mudo ligado: {rx.isMuted()}")
    
    rx.setMute(False)
    print(f"Mudo desligado: {rx.isMuted()}")
    
    # Testar busca
    print("\nTestando busca de estações...")
    rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP, callback=lambda: print(f"Buscando... {rx.getFrequency()/100:.1f} MHz"))
    
    print(f"Estação encontrada: {rx.getFrequency()/100:.1f} MHz")
    
    print("\nTeste concluído!")


if __name__ == "__main__":
    main()