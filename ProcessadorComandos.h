#ifndef PROCESSADOR_COMANDOS_H
#define PROCESSADOR_COMANDOS_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_system.h"
#include "ServicoUDP.h"
#include "DisplayOLED.h"
#include "Hardware.h"
#include "Configuracao.h"
#include "NTPUtil.h" // Inclui o cabeçalho do NTP

class ProcessadorComandos {
private:
    ServicoUDP& udp;
    DisplayOLED& oled;
    Hardware& hardware;
    Configuracao& config;
    NTPUtil& ntp; // Referência para acessar o relógio do sistema

    void executar(String cmd);

public:
    // Atualizado: Construtor agora recebe a referência do ntp
    ProcessadorComandos(ServicoUDP& srvUdp, DisplayOLED& display, Hardware& hw, Configuracao& cfg, NTPUtil& ntpService);
    void processar();
    void resetarSistemaFabrica();
};

#endif
