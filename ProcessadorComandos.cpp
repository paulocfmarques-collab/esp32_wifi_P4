#include "ProcessadorComandos.h"

ProcessadorComandos::ProcessadorComandos(ServicoUDP& srvUdp, DisplayOLED& display, Hardware& hw, Configuracao& cfg, NTPUtil& ntpService)
    : udp(srvUdp), oled(display), hardware(hw), config(cfg), ntp(ntpService) {} // Inicializa o NTP

void ProcessadorComandos::processar() {
    String comando;
    if (udp.checarPacote(comando)) {
        executar(comando);
    }
}

void ProcessadorComandos::resetarSistemaFabrica() {
    oled.adicionarLinha("Limpando Memoria...");
    udp.responder("WiFi zerado. Reiniciando...\n");
    config.limparTudo();
    hardware.piscarLedSync(10, 100);
    ESP.restart();
}

void ProcessadorComandos::executar(String cmd) {
    oled.adicionarLinha("> " + cmd);

    if (cmd == "RESET_WIFI") {
        resetarSistemaFabrica();
    }
    else if (cmd.startsWith("TIME")) {
        int p = cmd.indexOf(':');
        
        // Caso 1: Comando possui parâmetros dinâmicos (Ex: TIME:-5 ou TIME:2)
        if (p > 0) {
            int novoFuso = cmd.substring(p + 1).toInt();
            
            // Validação de segurança de fusos válidos (GMT-12 até GMT+14)
            if (novoFuso >= -12 && novoFuso <= 14) {
                config.salvarFusoHorario(novoFuso);
                ntp.atualizarFusoHorario(novoFuso);
                
                String confirma = "Fuso alterado para GMT" + String(novoFuso >= 0 ? "+" : "") + String(novoFuso);
                oled.adicionarLinha(confirma);
                udp.responder(confirma + "\n");
            } else {
                udp.responder("Erro: Fuso invalido. Use valores entre -12 e 14.\n");
            }
        } 
        // Caso 2: Chamada simples do comando apenas para ler a hora
        else {
            String dataHoraCompleta;
            ntp.getDateTime(dataHoraCompleta, 100);
            
            String hora = "--:--:--";
            if (dataHoraCompleta.length() >= 19) {
                hora = dataHoraCompleta.substring(11);
            }
            
            int fusoAtual = config.obterFusoHorario();
            String strFuso = " (GMT" + String(fusoAtual >= 0 ? "+" : "") + String(fusoAtual) + ")";
            
            oled.adicionarLinha("Hora: " + hora);
            udp.responder("Hora atual: " + hora + strFuso + "\n");
        }
    }
    else if (cmd == "DATE") {
        String dataHoraCompleta;
        ntp.getDateTime(dataHoraCompleta, 100);
        
        String data = "Erro NTP";
        if (dataHoraCompleta.length() >= 19) {
            data = dataHoraCompleta.substring(0, 10); // Recorta apenas "DD/MM/AAAA"
        }
        
        oled.adicionarLinha("Data: " + data);
        udp.responder("Data atual: " + data + "\n");
    }
    else if (cmd == "LED_ON") {
        hardware.ligarLed();
        udp.responder("LED ligado\n");
    }
    else if (cmd == "LED_OFF") {
        hardware.desligarLed();
        udp.responder("LED desligado\n");
    }
    else if (cmd == "TEMP") {
        float t = hardware.lerTemperatura();
        oled.adicionarLinha("Temp: " + String(t) + "C");
        udp.responderPrintf("CPU Temp: %.2f\n", t);
    }
    else if (cmd == "CPU") {
        udp.responderPrintf("Modelo: %s\nRevisao: %d\nNucleos: %d\nCPU: %d MHz\nRAM livre: %u bytes\n",
            ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz(), ESP.getFreeHeap());
    }
    else if (cmd == "RAM") {
        udp.responderPrintf("Heap livre: %u\nMenor heap livre: %u\nMaior bloco livre: %u\n",
            ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    }
    else if (cmd == "FLASH") {
        udp.responderPrintf("Flash total: %u\nVelocidade Flash: %u\nTamanho Sketch: %u\nEspaco livre: %u\n",
            ESP.getFlashChipSize(), ESP.getFlashChipSpeed(), ESP.getSketchSize(), ESP.getFreeSketchSpace());
    }
    else if (cmd == "INIT") {
        udp.responderPrintf("Motivo reset: %d\n", esp_reset_reason());
    }
    else if (cmd == "UPTIME") {
        udp.responderPrintf("Uptime: %lu ms\n", millis());
    }
    else if (cmd == "MAC") {
        udp.responderPrintf("MAC: %s\n", WiFi.macAddress().c_str());
    }
    else if (cmd == "NET_INFO") {
        udp.responderPrintf("IP: %s\nGateway: %s\nMascara: %s\nRSSI: %d dbm\nSSID: %s\n",
            WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(), 
            WiFi.subnetMask().toString().c_str(), WiFi.RSSI(), WiFi.SSID().c_str());
    }  
    else if (cmd.startsWith("LED_PISCA")) {
        int piscadas = 10, tempo = 250;
        int p1 = cmd.indexOf(':'), p2 = cmd.indexOf(':', p1 + 1);
        if (p1 > 0 && p2 > 0) {
            piscadas = cmd.substring(p1 + 1, p2).toInt();
            tempo = cmd.substring(p2 + 1).toInt();
        }
        hardware.piscarLedSync(piscadas, tempo);
        udp.responderPrintf("LED piscou %d vezes com %d ms\n", piscadas, tempo);
    }
    else if (cmd.startsWith("LED_BLINK")) {
        int p = cmd.indexOf(':');
        int intervalo = 500;
        if (p > 0) intervalo = cmd.substring(p + 1).toInt();
        hardware.iniciarBlinkAsync(intervalo);
        udp.responderPrintf("Blink iniciado (%d ms)\n", intervalo);
    }
    else {
        udp.responder("Comando Invalido\n");
    }
}
