#include "ConexaoWiFi.h"

const char htmlPageWiFi[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="UTF-8"><title>Configuração WiFi</title></head>
<body style="font-family:Arial;text-align:center;background:#f4f4f9;">
  <div style="background:white;max-width:300px;margin:40px auto;padding:20px;border-radius:8px;box-shadow:0 4px 8px rgba(0,0,0,0.1);">
    <h2>Configuração WiFi - ESP32-P4</h2>
    <form action="/salvar" method="POST">
      <input type="text" name="ssid" placeholder="Nome da rede" style="width:100%;padding:8px;margin:10px 0;" required><br>
      <input type="password" name="senha" placeholder="Senha da rede" style="width:100%;padding:8px;margin:10px 0;"><br>
      <input type="submit" value="Salvar" style="width:100%;padding:8px;background:#007bff;color:white;border:none;cursor:pointer;">
    </form>
  </div>
</body></html>
)rawliteral";

ConexaoWiFi::ConexaoWiFi(Configuracao& cfg, DisplayOLED& display, Hardware& hw) 
    : server(80), config(cfg), oled(display), hardware(hw) {}

void ConexaoWiFi::inicializar() {
    String ssid, senha;
    if (config.obterCredenciais(ssid, senha)) {
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true); // Ativa o recurso nativo do ESP32 de auto-reconexão
        WiFi.begin(ssid.c_str(), senha.c_str());
        oled.adicionarLinha("Conectando a:");
        oled.adicionarLinha(ssid);

        int tentativas = 0;
        // Limita as tentativas iniciais para não travar o boot para sempre
        while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
            delay(500);
            hardware.alternarLed();
            tentativas++;
        }
        hardware.desligarLed();

        if (WiFi.status() == WL_CONNECTED) {
            oled.adicionarLinha("WiFi Conectado!");
            oled.adicionarLinha(WiFi.localIP().toString());
            return;
        } else {
            oled.adicionarLinha("Falha inicial WiFi.");
        }
    }
    // Se falhar ou não tiver dados salvos, abre o portal de configuração
    iniciarPortal();
}

void ConexaoWiFi::iniciarPortal() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_P4_CONFIG");
    oled.adicionarLinha("Portal Ativo!");
    oled.adicionarLinha("SSID: ESP32_P4_CONFIG");
    oled.adicionarLinha("IP: 192.168.4.1");

    server.on("/", HTTP_GET, [this]() { server.send(200, "text/html", htmlPageWiFi); });
    server.on("/salvar", HTTP_POST, [this]() {
        oled.adicionarLinha("Salvando rede...");
        config.salvarCredenciais(server.arg("ssid"), server.arg("senha"));
        server.send(200, "text/html", "<h2>Configuracao salva! Reiniciando...</h2>");
        delay(2000);
        ESP.restart();
    });
    server.begin();
}

void ConexaoWiFi::processar() {
    if (WiFi.getMode() == WIFI_AP) {
        server.handleClient();
    } 
    else if (WiFi.getMode() == WIFI_STA) {
        static unsigned long ultimaVerificacao = 0;
        unsigned long agora = millis();
        
        // Monitora o status a cada 10 segundos sem usar delay()
        if (agora - ultimaVerificacao >= 10000) {
            ultimaVerificacao = agora;
            if (WiFi.status() != WL_CONNECTED) {
                oled.adicionarLinha("WiFi Perdido! Tentando...");
                // Força uma reconexão caso a pilha interna do ESP tenha estagnado
                WiFi.disconnect();
                WiFi.reconnect();
            }
        }
    }
}

bool ConexaoWiFi::estaConectado() { return (WiFi.status() == WL_CONNECTED); }
