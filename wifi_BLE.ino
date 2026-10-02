#include "DisplayOLED.h"
#include "Configuracao.h"
#include "Hardware.h"
#include "ConexaoWiFi.h"
#include "ServicoUDP.h"
#include "ProcessadorComandos.h"
#include "NTPUtil.h"

DisplayOLED oled;
Configuracao configMngr;
Hardware hw;
NTPUtil ntp; 

ConexaoWiFi wifi(configMngr, oled, hw);
ServicoUDP udpService(4210);

// Atualizado: repassando a referência do objeto ntp aqui
ProcessadorComandos comandos(udpService, oled, hw, configMngr, ntp); 

bool udpInicializado = false;

void setup() {
    Serial.begin(115200);
    delay(500); // Estabilização das linhas de energia do ESP32-P4

    hw.inicializar();
    oled.inicializar();
    wifi.inicializar();

    if (wifi.estaConectado()) {
        udpService.iniciar();
        udpInicializado = true;
        
        // Recupera o último fuso salvo e inicializa o NTP com ele
        int fusoSalvo = configMngr.obterFusoHorario();
        ntp.initNTP(fusoSalvo);
    }
}


void loop() {
    wifi.processar();
    
    if (wifi.estaConectado()) {
        if (!udpInicializado) {
            udpService.iniciar();
            udpInicializado = true;
        }
        comandos.processar(); 
    } else {
        udpInicializado = false;
    }

    hw.atualizarBlink();
    
    oled.atualizarTela(ntp);

    if (hw.botaoResetPressionado()) {
        comandos.resetarSistemaFabrica();
    }
}
