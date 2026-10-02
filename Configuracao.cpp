#include "Configuracao.h"

Configuracao::Configuracao() {}

bool Configuracao::obterCredenciais(String &ssid, String &senha) {
    prefs.begin("wifi", true);
    ssid = prefs.getString("ssid", "");
    senha = prefs.getString("senha", "");
    prefs.end();
    return (ssid != "");
}

void Configuracao::salvarCredenciais(String ssid, String senha) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("senha", senha);
    prefs.end();
}

void Configuracao::limparTudo() {
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
}

int Configuracao::obterFusoHorario() {
    prefs.begin("wifi", true);
    int fuso = prefs.getInt("fuso", -3); // Default -3 se não existir
    prefs.end();
    return fuso;
}

void Configuracao::salvarFusoHorario(int fuso) {
    prefs.begin("wifi", false);
    prefs.putInt("fuso", fuso);
    prefs.end();
}

