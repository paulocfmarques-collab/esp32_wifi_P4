#ifndef CONEXAO_WIFI_H
#define CONEXAO_WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Configuracao.h"
#include "DisplayOLED.h"
#include "Hardware.h"

class ConexaoWiFi {
private:
    WebServer server;
    Configuracao& config;
    DisplayOLED& oled;
    Hardware& hardware;
    bool rotasOk;

    void iniciarPortal();
    void configurarRotas();
    bool tentarRede(const String& ssid, const String& senha);
    bool conectarListaSalva();
    void paginaConfig();
    void paginaInfo();
    void salvarForm();

public:
    ConexaoWiFi(Configuracao& cfg, DisplayOLED& display, Hardware& hw);
    void inicializar();
    void processar();
    bool estaConectado();
};

#endif
