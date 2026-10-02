#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <Arduino.h>
#include <Preferences.h>

class Configuracao {
private:
    Preferences prefs;

public:
    Configuracao();
    bool obterCredenciais(String &ssid, String &senha);
    void salvarCredenciais(String ssid, String senha);
    void limparTudo();
    int obterFusoHorario();
    void salvarFusoHorario(int fuso);
};

#endif
