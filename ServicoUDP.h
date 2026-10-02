#ifndef SERVICO_UDP_H
#define SERVICO_UDP_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <WiFi.h>

class ServicoUDP {
private:
    WiFiUDP udp;
    const int porta;

public:
    ServicoUDP(int p = 4210);
    void iniciar();
    bool checarPacote(String &mensagem);
    void responder(String resposta);
    void responderPrintf(const char* format, ...);
};

#endif
