#include "ConexaoWiFi.h"

static String esc(const String& s) {
    String o;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (c == '&') o += "&amp;";
        else if (c == '<') o += "&lt;";
        else if (c == '>') o += "&gt;";
        else if (c == '"') o += "&quot;";
        else o += c;
    }
    return o;
}

static const char CSS_WIFI[] PROGMEM =
    "<style>body{font-family:Arial;margin:20px;background:#f4f4f9;text-align:center;}"
    ".box{background:white;max-width:420px;margin:auto;padding:20px;border-radius:8px;box-shadow:0 4px 8px rgba(0,0,0,0.1);text-align:left;}"
    "input{width:100%;box-sizing:border-box;margin:3px 0;padding:6px;}"
    "td{padding:3px 6px;}a{color:#06c;}</style>";

ConexaoWiFi::ConexaoWiFi(Configuracao& cfg, DisplayOLED& display, Hardware& hw)
    : server(80), config(cfg), oled(display), hardware(hw), rotasOk(false) {}

bool ConexaoWiFi::tentarRede(const String& ssid, const String& senha) {
    oled.adicionarLinha("Conectando a:");
    oled.adicionarLinha(ssid);
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), senha.c_str());

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 24) {
        delay(500);
        hardware.alternarLed();
        tentativas++;
    }
    hardware.desligarLed();
    return WiFi.status() == WL_CONNECTED;
}

bool ConexaoWiFi::conectarListaSalva() {
    String ssids[NUM_REDES], senhas[NUM_REDES];
    int idx;
    config.carregarRedes(ssids, senhas, idx);

    bool algumaSalva = false;
    for (int i = 0; i < NUM_REDES; i++) if (ssids[i] != "") algumaSalva = true;
    if (!algumaSalva) return false;

    WiFi.mode(WIFI_STA);
    oled.adicionarLinha("Buscando redes...");
    int n = WiFi.scanNetworks();

    for (int k = 1; k <= NUM_REDES; k++) {
        int i = ((idx + k) % NUM_REDES + NUM_REDES) % NUM_REDES;
        if (ssids[i] == "") continue;

        bool visivel = false;
        for (int j = 0; j < n; j++) {
            if (WiFi.SSID(j) == ssids[i]) { visivel = true; break; }
        }
        if (!visivel) continue;

        if (tentarRede(ssids[i], senhas[i])) {
            config.salvarIndiceRede(i);
            WiFi.scanDelete();
            return true;
        }
    }
    WiFi.scanDelete();
    return false;
}

void ConexaoWiFi::inicializar() {
    if (conectarListaSalva()) {
        WiFi.setAutoReconnect(true);
        oled.adicionarLinha("WiFi Conectado!");
        oled.adicionarLinha(WiFi.localIP().toString());
        configurarRotas();
        return;
    }
    oled.adicionarLinha("Nenhuma rede da lista.");
    iniciarPortal();
}

void ConexaoWiFi::configurarRotas() {
    if (rotasOk) return;
    server.on("/", HTTP_GET, [this]() {
        if (WiFi.getMode() == WIFI_AP) paginaConfig(); else paginaInfo();
    });
    server.on("/info", HTTP_GET, [this]() { paginaInfo(); });
    server.on("/wifi", HTTP_GET, [this]() { paginaConfig(); });
    server.on("/salvar", HTTP_POST, [this]() { salvarForm(); });
    server.begin();
    rotasOk = true;
}

void ConexaoWiFi::iniciarPortal() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_P4_CONFIG");
    oled.adicionarLinha("Portal Ativo!");
    oled.adicionarLinha("SSID: ESP32_P4_CONFIG");
    oled.adicionarLinha("IP: 192.168.4.1");
    configurarRotas();
}

void ConexaoWiFi::paginaConfig() {
    String ssids[NUM_REDES], senhas[NUM_REDES];
    int idx;
    config.carregarRedes(ssids, senhas, idx);

    String h = F("<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>WiFi - ESP32-P4</title>");
    h += FPSTR(CSS_WIFI);
    h += F("</head><body><div class='box'><h2>Configuração WiFi - ESP32-P4</h2><form action='/salvar' method='POST'>");
    for (int i = 0; i < NUM_REDES; i++) {
        h += "<b>Rede " + String(i + 1) + "</b><input type='text' name='ssid" + String(i) + "' placeholder='SSID' value='" + esc(ssids[i]) + "'>";
        h += "<input type='password' name='senha" + String(i) + "' placeholder='" + String(ssids[i] != "" ? "(manter senha atual)" : "Senha") + "'>";
    }
    h += F("<br><input type='submit' value='Salvar e Reiniciar'></form><p><a href='/info'>Informações</a></p></div></body></html>");
    server.send(200, "text/html", h);
}

void ConexaoWiFi::paginaInfo() {
    String ssids[NUM_REDES], senhas[NUM_REDES];
    int idx;
    config.carregarRedes(ssids, senhas, idx);

    uint32_t seg = millis() / 1000;
    String h = F("<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><meta http-equiv='refresh' content='10'><title>Info - ESP32-P4</title>");
    h += FPSTR(CSS_WIFI);
    h += F("</head><body><div class='box'><h2>ESP32-P4 - Informações</h2><table>");
    h += "<tr><td>Chip</td><td>" + String(ESP.getChipModel()) + " rev " + String(ESP.getChipRevision()) + "</td></tr>";
    h += "<tr><td>SSID</td><td>" + esc(WiFi.SSID()) + "</td></tr>";
    h += "<tr><td>IP</td><td>" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String("-")) + "</td></tr>";
    h += "<tr><td>RSSI</td><td>" + String(WiFi.RSSI()) + " dBm</td></tr>";
    h += "<tr><td>MAC</td><td>" + WiFi.macAddress() + "</td></tr>";
    h += "<tr><td>Heap livre</td><td>" + String(ESP.getFreeHeap() / 1024) + " KB</td></tr>";
    h += "<tr><td>Uptime</td><td>" + String(seg / 3600) + "h " + String((seg / 60) % 60) + "m " + String(seg % 60) + "s</td></tr>";
    h += F("</table><h3>Redes salvas</h3><ol>");
    for (int i = 0; i < NUM_REDES; i++) {
        if (ssids[i] == "") continue;
        h += "<li>" + esc(ssids[i]) + (i == idx ? " (última usada)" : "") + "</li>";
    }
    h += F("</ol><p><a href='/wifi'>Configurar redes</a></p></div></body></html>");
    server.send(200, "text/html", h);
}

void ConexaoWiFi::salvarForm() {
    String ssids[NUM_REDES], senhas[NUM_REDES];
    int idx;
    config.carregarRedes(ssids, senhas, idx);

    String novosS[NUM_REDES], novasP[NUM_REDES];
    for (int i = 0; i < NUM_REDES; i++) {
        String novo = server.arg("ssid" + String(i));
        String senha = server.arg("senha" + String(i));
        novo.trim();
        if (novo == "") continue;
        novosS[i] = novo;
        novasP[i] = (senha == "" && novo == ssids[i]) ? senhas[i] : senha;
    }
    oled.adicionarLinha("Salvando redes...");
    config.salvarRedes(novosS, novasP);
    server.send(200, "text/html", "<meta charset='UTF-8'><h2>Configuração salva! Reiniciando...</h2>");
    delay(2000);
    ESP.restart();
}

void ConexaoWiFi::processar() {
    server.handleClient();

    if (WiFi.getMode() == WIFI_STA) {
        static unsigned long ultimaVerificacao = 0;
        unsigned long agora = millis();

        // Monitora o status a cada 10 segundos sem usar delay()
        if (agora - ultimaVerificacao >= 10000) {
            ultimaVerificacao = agora;
            if (WiFi.status() != WL_CONNECTED) {
                oled.adicionarLinha("WiFi Perdido! Tentando...");
                WiFi.disconnect();
                WiFi.reconnect();
            }
        }
    }
}

bool ConexaoWiFi::estaConectado() { return (WiFi.status() == WL_CONNECTED); }
