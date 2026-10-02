#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "NTPUtil.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MAX_LINHAS 8
#define PIN_SDA 7
#define PIN_SCL 8

class DisplayOLED {
private:
    Adafruit_SSD1306 display;
    String historicoLinhas[MAX_LINHAS];
    int totalLinhas;
    bool inicializado;
    
    // Variáveis de controle interno do tempo de tela
    bool modoComando;
    unsigned long timestampComando;
    const unsigned long TEMPO_EXIBICAO = 10000; // 10 segundos

    void renderizarHistorico();
    void renderizarRelogio(NTPUtil& ntp); // Declarada aqui para o .cpp reconhecer

public:
    DisplayOLED();
    bool inicializar();
    void adicionarLinha(String novoTexto);
    void limpar();
    void atualizarTela(NTPUtil& ntp); 
};

#endif
