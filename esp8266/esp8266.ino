//#include <NTPClient.h>
//#include <WiFiUdp.h>

#include <ESP8266WiFi.h>

#include <FS.h>
#include <LittleFS.h>

#include "config.hpp"
#include "log.hpp"
#include "webserver.hpp"

#include "osapi.h"

//WiFiUDP udp;
//NTPClient ntp(udp, "a.st1.ntp.br", 0, 60000); // Horário GMT

// aplicaConfig usada apenas no estado de configuração para alertar
// quando recarregar as novas configurações
bool aplicaConfig = false;

template <typename T>
void logError(const T str) {
    Serial.print(str);
    //logFile(str, ntp.getFormattedTime());
}

/***************************************
 * Gerenciamento de estado da aplicação
 ***************************************/
/*enum class Estado : int {
    INICIO,
    CONECTADO,
    DESCONECTADO,
    RECUPERACAO,
    CONFIGURACAO
};*/

//volatile Estado estadoAtual = Estado::INICIO;

/****************************************************************************
 * Callback do temporizador. Define quando uma aquisição deve ser registrada
 ****************************************************************************/
/*volatile bool registra = false;
os_timer_t timer;
void timer_func(void *) {
    if(estadoAtual != Estado::CONFIGURACAO)
        registra = true;
}*/

/***********************************************
 * Funções de configuração para chamar em setup
 ***********************************************/
WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
void setupWiFi() {
    WiFi.mode(WIFI_STA);
    //Serial.println("Conectando a " + mainConfig[KEY_STA_SSID]);
    //Serial.println("Senha " + mainConfig[KEY_STA_PASS]);
    WiFi.begin(mainConfig[KEY_STA_SSID].c_str(), mainConfig[KEY_STA_PASS].c_str());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    stationConnectedHandler = WiFi.onStationModeConnected(&onStationConnected);
    stationDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationDisconnected);

    unsigned long timePoint = millis();
    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);

        unsigned long now = millis();
        unsigned long delta = now - timePoint;
        if(delta >= 10000)
        {
            Serial.println("Iniciando ponto de acesso...");
            WiFi.mode(WIFI_AP);
            if (!WiFi.softAP(mainConfig[KEY_AP_SSID].c_str(), mainConfig[KEY_AP_PASS].c_str())) {
                Serial.println(ERRO_AP);
                ESP.restart();
            }
            break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    LittleFSConfig fsConfig{false};
    LittleFS.setConfig(fsConfig);
    //SPIFFSConfig fsConfig{false};
    //SPIFFS.setConfig(fsConfig);
    //if(!SPIFFS.begin())
    if(!LittleFS.begin())
    {
        Serial.println("Erro ao montar sistema de arquivos");
        delay(5000);
        ESP.restart();
    }

    // Carrega configurações do sistema de arquivos
    Serial.println(INFO_INI);
    if (!loadConfig()) {
        logError(ERRO_CONFIG);
        ESP.restart();
    }
    Serial.println(INFO_PRONTO);

    setupWiFi();
    setupWebserver();

    // Se o botão estiver pressionado durante a inicialização
    // entra no modo de configuração.
    // É criado um ponto de acesso e um servidor web para
    // ver/alterar as configurações.
    /*if(digitalRead(D5) == LOW || dadosPendentes) {
        WiFi.mode(WIFI_AP);
        if (!WiFi.softAP(mainConfig[KEY_AP_SSID].c_str(), mainConfig[KEY_AP_PASS].c_str())) {
            Serial.println(ERRO_AP);
            ESP.restart();
        }
        setupWebserver();
        estadoAtual = Estado::CONFIGURACAO;
        Serial.println("Modo de config");
    } else {
        setupWiFi();
        ntp.begin();
        ntp.update();
        if(ntp.getEpochTime() < 1687787861) {
            logError("Falha NTP");
            ESP.restart();
        }
        // Utlizada o temporizador de software para definir o
        // ritmo de aquisições
        os_timer_setfn(&timer, timer_func, nullptr);
        os_timer_arm(&timer, samplingPeriodMs, true);
    }*/
}

/*******************************************
 * Há um loop para cada estado da aplicação
 *******************************************/
//void loopDesconectado() {
//}

//void loopRecuperacao() {
//}

//void loopConectado() {
    // ntp.update();
//}

/*void loopConfiguracao() {
    if(aplicaConfig) {
        applyConfig();
        Serial.println(INFO_UPDATE);
        ESP.restart();
    }
}*/

void loop() {
    if(aplicaConfig)
    {
        applyConfig();
    }
    //ntp.update();
    /*switch(estadoAtual) {
        case Estado::DESCONECTADO:
            loopDesconectado();
            break;
        case Estado::RECUPERACAO:
            {
                loopRecuperacao();
            }
            break;
        case Estado::CONECTADO:
            {
                loopConectado();
            }
            break;
        case Estado::CONFIGURACAO:
            loopConfiguracao();
            break;
    }*/
}

void onStationConnected(const WiFiEventStationModeConnected& evt) {
    /*if(estadoAtual == Estado::DESCONECTADO) {
        Serial.println("Recuperando...");
        estadoAtual = Estado::RECUPERACAO;
    } else {
        Serial.println("Conectado!");
        estadoAtual = Estado::CONECTADO;
    }*/
}

void onStationDisconnected(const WiFiEventStationModeDisconnected& evt) {
    /*if(estadoAtual != Estado::INICIO) {
        Serial.println("Desconectado!");
        estadoAtual = Estado::DESCONECTADO;
    }*/
}
