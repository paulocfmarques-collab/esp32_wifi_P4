#include "Configuracao.h"

Configuracao::Configuracao() {}

void Configuracao::carregarRedes(String ssids[NUM_REDES], String senhas[NUM_REDES], int &idx) {
    prefs.begin("wifi", false);
    String antigo = prefs.getString("ssid", "");
    if (antigo != "") {
        if (prefs.getString("s0", "") == "") {
            prefs.putString("s0", antigo);
            prefs.putString("p0", prefs.getString("senha", ""));
        }
        prefs.remove("ssid");
        prefs.remove("senha");
    }
    for (int i = 0; i < NUM_REDES; i++) {
        ssids[i]  = prefs.getString(("s" + String(i)).c_str(), "");
        senhas[i] = prefs.getString(("p" + String(i)).c_str(), "");
    }
    idx = prefs.getInt("idx", -1);
    prefs.end();
}

void Configuracao::salvarRedes(const String ssids[NUM_REDES], const String senhas[NUM_REDES]) {
    prefs.begin("wifi", false);
    for (int i = 0; i < NUM_REDES; i++) {
        prefs.putString(("s" + String(i)).c_str(), ssids[i]);
        prefs.putString(("p" + String(i)).c_str(), senhas[i]);
    }
    prefs.putInt("idx", -1);
    prefs.end();
}

void Configuracao::salvarIndiceRede(int idx) {
    prefs.begin("wifi", false);
    prefs.putInt("idx", idx);
    prefs.end();
}

bool Configuracao::obterCredenciais(String &ssid, String &senha) {
    String s[NUM_REDES], p[NUM_REDES];
    int idx;
    carregarRedes(s, p, idx);
    for (int i = 0; i < NUM_REDES; i++) {
        if (s[i] != "") { ssid = s[i]; senha = p[i]; return true; }
    }
    return false;
}

void Configuracao::salvarCredenciais(String ssid, String senha) {
    prefs.begin("wifi", false);
    prefs.putString("s0", ssid);
    prefs.putString("p0", senha);
    prefs.putInt("idx", -1);
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
