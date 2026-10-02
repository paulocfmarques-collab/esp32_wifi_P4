#include "ServicoUDP.h"

ServicoUDP::ServicoUDP(int p) : porta(p) {}

void ServicoUDP::iniciar() { 
    udp.begin(porta); 
}

bool ServicoUDP::checarPacote(String &mensagem) {
    int packetSize = udp.parsePacket();
    
    // Tratamento de Erro: Ignora pacotes vazios ou maliciosamente gigantescos
    if (packetSize > 0 && packetSize < 256) {
        char buffer[256];
        // Proteção estrita contra estouro de array limitando os bytes lidos
        int len = udp.read(buffer, sizeof(buffer) - 1); 
        
        if (len > 0) {
            buffer[len] = '\0'; // Garante o terminador nulo da string em C
            mensagem = String(buffer);
            mensagem.trim();
            
            // Tratamento de Erro: Ignora comandos vazios (ex: apenas espaços ou \n)
            if (mensagem.length() == 0) {
                return false;
            }
            return true;
        }
    }
    return false;
}

void ServicoUDP::responder(String resposta) {
    // Tratamento de Erro: Só tenta enviar pacotes se o Wi-Fi estiver fisicamente conectado
    if (WiFi.status() != WL_CONNECTED) return;

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.print(resposta);
    udp.endPacket();
}

void ServicoUDP::responderPrintf(const char* format, ...) {
    if (WiFi.status() != WL_CONNECTED) return;

    char loc_buf[256];
    va_list arg;
    va_start(arg, format);
    // Tratamento de Erro: vsnprintf impede que dados variáveis estourem os 256 bytes do buffer
    int resultado = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(arg);

    if (resultado >= 0) {
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.print(loc_buf);
        udp.endPacket();
    }
}
