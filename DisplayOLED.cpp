#include "DisplayOLED.h"

DisplayOLED::DisplayOLED() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), 
      totalLinhas(0), inicializado(false), modoComando(false), timestampComando(0) {}

bool DisplayOLED::inicializar() {
    Wire.begin(PIN_SDA, PIN_SCL); 
    if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        inicializado = true;
        display.clearDisplay();
        display.display();
        adicionarLinha("OLED Pronto!");
        return true;
    }
    Serial.println("Falha ao encontrar o Display OLED!");
    return false;
}

void DisplayOLED::adicionarLinha(String novoTexto) {
    Serial.println("[OLED] " + novoTexto);
    if (!inicializado) return;

    // Quando chega um texto, ativa o modo comando e guarda o tempo atual
    modoComando = true;
    timestampComando = millis();

    if (totalLinhas >= MAX_LINHAS) {
        for (int i = 0; i < MAX_LINHAS - 1; i++) {
            historicoLinhas[i] = historicoLinhas[i + 1];
        }
        historicoLinhas[MAX_LINHAS - 1] = novoTexto;
    } else {
        historicoLinhas[totalLinhas] = novoTexto;
        totalLinhas++;
    }

    renderizarHistorico();
}

void DisplayOLED::renderizarHistorico() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    for (int i = 0; i < totalLinhas; i++) {
        display.setCursor(0, i * 8); 
        display.println(historicoLinhas[i]);
    }
    display.display();
}

void DisplayOLED::renderizarRelogio(NTPUtil& ntp) {
    String dataHoraCompleta;
    ntp.getDateTime(dataHoraCompleta, 100);

    String data = "Aguardando NTP...";
    String hora = "--:--:--";
    
    if (dataHoraCompleta.length() >= 19) {
        data = dataHoraCompleta.substring(0, 10);
        hora = dataHoraCompleta.substring(11);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    int16_t x1, y1;
    uint16_t w, h;

    // Desenha a Data
    display.setTextSize(1);
    display.getTextBounds(data, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 12);
    display.println(data);

    // Desenha a Hora
    display.setTextSize(2);
    display.getTextBounds(hora, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT / 2) - (h / 2) + 8);
    display.println(hora);

    display.display();
}

void DisplayOLED::limpar() {
    if (!inicializado) return;
    totalLinhas = 0;
    display.clearDisplay();
    display.display();
}

void DisplayOLED::atualizarTela(NTPUtil& ntp) {
    if (!inicializado) return;

    if (modoComando) {
        // Se passarem os 10 segundos, desativa e limpa a tela para o relógio entrar
        if (millis() - timestampComando >= TEMPO_EXIBICAO) {
            modoComando = false;
            limpar();
        }
    } else {
        static unsigned long ultimoUpdate = 0;
        if (millis() - ultimoUpdate >= 500) { 
            ultimoUpdate = millis();
            renderizarRelogio(ntp);
        }
    }
}
