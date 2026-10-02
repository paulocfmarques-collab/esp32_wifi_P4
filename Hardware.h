#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include "driver/temperature_sensor.h"

#define LED 1
#define BOTAO_RESET 2

class Hardware {
private:
    bool blinkAtivo;
    bool estadoLed;
    unsigned long ultimoToggle;
    unsigned long intervaloBlink;
    temperature_sensor_handle_t tempSensor;

public:
    Hardware();
    void inicializar();
    void atualizarBlink();
    
    void ligarLed();
    void desligarLed();
    void alternarLed();
    void piscarLedSync(int vezes, int tempoMs);
    void iniciarBlinkAsync(unsigned long intervaloMs);
    void pararBlinkAsync();
    
    bool botaoResetPressionado();
    float lerTemperatura();
};

#endif
