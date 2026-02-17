#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>
#include <map>

// Chaves da requisição HTTP
#define KEY_STA_SSID        F("WIFI_SSID")
#define KEY_STA_PASS        F("WIFI_PASS")
#define KEY_ADMIN_LOGIN     F("ADMIN_USER")
#define KEY_ADMIN_PASS      F("ADMIN_PASS")
#define KEY_AP_SSID         F("AP_SSID")
#define KEY_AP_PASS         F("AP_PASS")
#define NUM_PARAMETERS  6

#define CONFIG_FILE F("/config.txt")

extern std::map<String, String> mainConfig;

bool validateConfig(const std::map<String, String> &cfg);

bool loadConfig();

void applyConfig();

#endif // CONFIG_HPP
