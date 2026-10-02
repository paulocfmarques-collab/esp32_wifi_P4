#include "Hardware.h"

Hardware::Hardware() 
    : blinkAtivo(false), estadoLed(false), ultimoToggle(0), intervaloBlink(500), tempSensor(NULL) {}

void Hardware::inicializar() {
    pinMode(LED, OUTPUT);
    pinMode(BOTAO_RESET, INPUT_PULLUP);
    desligarLed();

    temperature_sensor_config_t temp_cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    temperature_sensor_install(&temp_cfg, &tempSensor);
}

void Hardware::ligarLed() { blinkAtivo = false; estadoLed = true; digitalWrite(LED, HIGH); }
void Hardware::desligarLed() { blinkAtivo = false; estadoLed = false; digitalWrite(LED, LOW); }
void Hardware::alternarLed() { estadoLed = !estadoLed; digitalWrite(LED, estadoLed); }

void Hardware::piscarLedSync(int vezes, int tempoMs) {
    blinkAtivo = false;
    for (int i = 0; i < vezes; i++) {
        digitalWrite(LED, HIGH); delay(tempoMs);
        digitalWrite(LED, LOW);  delay(tempoMs);
    }
}

void Hardware::iniciarBlinkAsync(unsigned long intervaloMs) {
    intervaloBlink = intervaloMs;
    blinkAtivo = true;
}

void Hardware::pararBlinkAsync() { blinkAtivo = false; }

void Hardware::atualizarBlink() {
    if (!blinkAtivo) return;
    unsigned long atual = millis();
    if (atual - ultimoToggle >= intervaloBlink) {
        ultimoToggle = atual;
        alternarLed();
    }
}

bool Hardware::botaoResetPressionado() {
    if (digitalRead(BOTAO_RESET) == LOW) {
        delay(50);
        return (digitalRead(BOTAO_RESET) == LOW);
    }
    return false;
}

float Hardware::lerTemperatura() {
    if (tempSensor == NULL) return 0.0;
    float tsens_out;
    temperature_sensor_enable(tempSensor);
    temperature_sensor_get_celsius(tempSensor, &tsens_out);
    temperature_sensor_disable(tempSensor);
    return tsens_out;
}
