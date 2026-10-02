#include "NTPUtil.h"
#include <time.h>

bool NTPUtil::initNTP(int fusoInicial)
{
    Serial.println("--- Inicializando NTP ---");
    atualizarFusoHorario(fusoInicial);

    Serial.println("[NTP] Serviço iniciado. Aguardando sincronização...");

    for (int tentativa = 0; tentativa < 20; tentativa++)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 1000))
        {
            Serial.println("\n[NTP] Sincronização concluída.");
            return true;
        }
        delay(1000);
    }
    return false;
}

void NTPUtil::atualizarFusoHorario(int novoFuso) 
{
    char tzString[32];
    
    // No padrão POSIX do ESP32, o sinal é INVERTIDO.
    // Se o fuso é -3 (Brasília), a string deve ser "GMT3"
    // Se o fuso é +2 (Europa), a string deve ser "GMT-2"
    int fusoInvertido = -novoFuso;

    if (fusoInvertido >= 0) {
        snprintf(tzString, sizeof(tzString), "GMT%d", fusoInvertido);
    } else {
        snprintf(tzString, sizeof(tzString), "GMT-%d", -fusoInvertido);
    }
    
    Serial.printf("[NTP] Aplicando String de Fuso POSIX Correta: %s\n", tzString);
    
    // Aplica o fuso e aponta para os servidores NTP estáveis
    configTzTime(
        tzString,
        "a.st1.ntp.br",
        "pool.ntp.org",
        "time.nist.gov"
    );
}

void NTPUtil::getDateTime(String& dateTime, uint32_t timeoutMs)
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, timeoutMs))
    {
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
        dateTime = String(buffer);
    }
    else
    {
        dateTime = "Erro ao obter data e hora";
    }
}
