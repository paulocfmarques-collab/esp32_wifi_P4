#include "ProcessadorComandos.h"

#define FIRMWARE_VERSION "v1.0.0"

static String _ultimoComando = "Nenhum";
static uint32_t _totalComandos = 0;

static const char* motivoReset() {
    switch (esp_reset_reason()) {
        case ESP_RST_POWERON:   return "Power-on";
        case ESP_RST_EXT:       return "Reset externo";
        case ESP_RST_SW:        return "Reset por software";
        case ESP_RST_PANIC:     return "Panic";
        case ESP_RST_INT_WDT:   return "Watchdog interrupcao";
        case ESP_RST_TASK_WDT:  return "Watchdog tarefa";
        case ESP_RST_WDT:       return "Watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep sleep";
        case ESP_RST_BROWNOUT:  return "Brownout";
        default:                return "Desconhecido";
    }
}

ProcessadorComandos::ProcessadorComandos(ServicoUDP& srvUdp, DisplayOLED& display, Hardware& hw, Configuracao& cfg, NTPUtil& ntpService)
    : udp(srvUdp), oled(display), hardware(hw), config(cfg), ntp(ntpService) {}

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
    cmd.trim();

    String arg = "";
    String nome = cmd;
    int sep = cmd.indexOf(':');
    if (sep != -1) {
        nome = cmd.substring(0, sep);
        arg = cmd.substring(sep + 1);
    }
    nome.trim();
    nome.toLowerCase();

    // Apelidos dos nomes antigos
    if (nome == "init") nome = "reason";
    else if (nome == "restart") nome = "reboot";
    else if (nome == "set_tz") nome = "set_fuso";

    if (nome != "lastcmd" && nome != "cmdcount") {
        _ultimoComando = cmd;
        _totalComandos++;
    }

    oled.adicionarLinha("> " + cmd);

    // ─── Sistema ───
    if (nome == "lastcmd") {
        udp.responder("Ultimo Comando: " + _ultimoComando + "\n");
    }
    else if (nome == "cmdcount") {
        udp.responderPrintf("Total Comandos: %u\n", (unsigned)_totalComandos);
    }
    else if (nome == "alive") {
        udp.responder("ALIVE\n");
    }
    else if (nome == "version") {
        udp.responder("Versao Firmware: " FIRMWARE_VERSION "\n");
    }
    else if (nome == "build") {
        udp.responderPrintf("Compilacao:\nData: %s\nHora: %s\n", __DATE__, __TIME__);
    }
    else if (nome == "reason") {
        udp.responder(String("Reset: ") + motivoReset() + "\n");
    }
    else if (nome == "info") {
        udp.responderPrintf(
            "===== DEVICE INFO =====\nFirmware: %s\nBuild: %s %s\nSSID: %s\nIP: %s\nMAC: %s\nRSSI: %d dBm\nHeap Livre: %u KB\nUptime: %lu ms\n=======================\n",
            FIRMWARE_VERSION, __DATE__, __TIME__, WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
            WiFi.macAddress().c_str(), WiFi.RSSI(), (unsigned)(ESP.getFreeHeap() / 1024), millis());
    }
    else if (nome == "status") {
        udp.responderPrintf("ONLINE\nWiFi: %s\nHeap: %u KB\n",
            WiFi.status() == WL_CONNECTED ? "OK" : "FALHA", (unsigned)(ESP.getFreeHeap() / 1024));
    }
    else if (nome == "help") {
        udp.responder(
            "Sistema: info status version build reason help reboot reset_wifi alive lastcmd cmdcount\n"
            "Hardware: temp cpu ram heap flash uptime\n"
            "Rede: net_info mac rssi ip ssid\n"
            "Hora: time date set_fuso:N\n"
            "LED: led_on led_off led_blink[:ms] led_pisca[:n:ms]\n");
    }
    else if (nome == "reset_wifi") {
        resetarSistemaFabrica();
    }
    else if (nome == "reboot") {
        oled.adicionarLinha("Reiniciando...");
        udp.responder("Reiniciando...\n");
        delay(1000);
        ESP.restart();
    }
    // ─── Hardware ───
    else if (nome == "temp") {
        float t = hardware.lerTemperatura();
        oled.adicionarLinha("Temp: " + String(t) + "C");
        udp.responderPrintf("Temp: %.2fC\n", t);
    }
    else if (nome == "cpu") {
        udp.responderPrintf("Modelo: %s\nRevisao: %d\nNucleos: %d\nFreq: %d MHz\n",
            ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
    }
    else if (nome == "ram") {
        udp.responderPrintf("Heap livre: %u\nMenor heap: %u\nMaior bloco: %u\n",
            ESP.getFreeHeap(), ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    }
    else if (nome == "heap") {
        udp.responderPrintf("Heap livre: %u B\n", ESP.getFreeHeap());
    }
    else if (nome == "flash") {
        udp.responderPrintf("Flash total: %u\nVelocidade Flash: %u\nTamanho Sketch: %u\nEspaco livre: %u\n",
            ESP.getFlashChipSize(), ESP.getFlashChipSpeed(), ESP.getSketchSize(), ESP.getFreeSketchSpace());
    }
    else if (nome == "uptime") {
        udp.responderPrintf("Uptime: %lu ms\n", millis());
    }
    // ─── Rede ───
    else if (nome == "net_info") {
        udp.responderPrintf("IP: %s\nGateway: %s\nMascara: %s\nRSSI: %d dBm\nSSID: %s\n",
            WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(),
            WiFi.subnetMask().toString().c_str(), WiFi.RSSI(), WiFi.SSID().c_str());
    }
    else if (nome == "mac") {
        udp.responderPrintf("MAC: %s\n", WiFi.macAddress().c_str());
    }
    else if (nome == "rssi") {
        udp.responderPrintf("RSSI: %d dBm\n", WiFi.RSSI());
    }
    else if (nome == "ip") {
        udp.responderPrintf("IP: %s\n", WiFi.localIP().toString().c_str());
    }
    else if (nome == "ssid") {
        udp.responderPrintf("SSID: %s\n", WiFi.SSID().c_str());
    }
    // ─── Hora ───
    else if (nome == "set_fuso" || (nome == "time" && sep != -1)) {
        int novoFuso = arg.toInt();
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
    else if (nome == "time") {
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
    else if (nome == "date") {
        String dataHoraCompleta;
        ntp.getDateTime(dataHoraCompleta, 100);

        String data = "Erro NTP";
        if (dataHoraCompleta.length() >= 19) {
            data = dataHoraCompleta.substring(0, 10);
        }

        oled.adicionarLinha("Data: " + data);
        udp.responder("Data atual: " + data + "\n");
    }
    // ─── LED ───
    else if (nome == "led_on") {
        hardware.pararBlinkAsync();
        hardware.ligarLed();
        udp.responder("LED ligado\n");
    }
    else if (nome == "led_off") {
        hardware.pararBlinkAsync();
        hardware.desligarLed();
        udp.responder("LED desligado\n");
    }
    else if (nome == "led_pisca") {
        int piscadas = 10, tempo = 250;
        int p = arg.indexOf(':');
        if (p > 0) {
            piscadas = arg.substring(0, p).toInt();
            tempo = arg.substring(p + 1).toInt();
        }
        hardware.piscarLedSync(piscadas, tempo);
        udp.responderPrintf("LED piscou %d vezes com %d ms\n", piscadas, tempo);
    }
    else if (nome == "led_blink") {
        int intervalo = 500;
        if (sep != -1 && arg.toInt() > 0) intervalo = arg.toInt();
        hardware.iniciarBlinkAsync(intervalo);
        udp.responderPrintf("Blink iniciado (%d ms)\n", intervalo);
    }
    else {
        udp.responder("Comando Invalido\n");
    }
}
