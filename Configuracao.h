#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <Arduino.h>
#include <Preferences.h>

#define NUM_REDES 5

class Configuracao {
private:
    Preferences prefs;

public:
    Configuracao();
    bool obterCredenciais(String &ssid, String &senha);
    void salvarCredenciais(String ssid, String senha);
    void carregarRedes(String ssids[NUM_REDES], String senhas[NUM_REDES], int &idx);
    void salvarRedes(const String ssids[NUM_REDES], const String senhas[NUM_REDES]);
    void salvarIndiceRede(int idx);
    void limparTudo();
    int obterFusoHorario();
    void salvarFusoHorario(int fuso);
};

#endif
