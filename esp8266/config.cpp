#include "config.hpp"
#include "log.hpp"

#include <FS.h>
#include <LittleFS.h>

std::map<String, String> mainConfig;
bool validateConfig(const std::map<String, String> &cfg) {
    if (cfg.size() == NUM_PARAMETERS) {
        for (const auto &it : cfg)
            if (it.second.length() == 0) return false;

        return true;
    }
    return false;
}

bool loadConfig() {
    File file = LittleFS.open(CONFIG_FILE, "r");
    //File file = SPIFFS.open(CONFIG_FILE, "r");
    String line;
    delay(1000);
    Serial.println("Lendo configs");
    if (file) {
        while ((line = file.readStringUntil('\n')).length() > 0) {
            line.trim();
            delay(200);
            int index;
            if ((index = line.indexOf('=')) > 0) {
                String key = line.substring(0, index);
                key.trim();
                String value = line.substring(index + 1);
                value.trim();
                mainConfig[key] = (value);
            }
        }
        file.close();
        return validateConfig(mainConfig);
    }
    Serial.println("Arquivo inexistente!");
    delay(1000);
    return false;
}

void applyConfig() {
    Serial.println("Aplicando novas configurações...");
    File file = LittleFS.open(CONFIG_FILE, "w");
    file.println(String(KEY_STA_SSID) + "=" + mainConfig[KEY_STA_SSID]);
    file.println(String(KEY_STA_PASS )+ "=" + mainConfig[KEY_STA_PASS]);
    file.println(String(KEY_ADMIN_LOGIN) + "=" + mainConfig[KEY_ADMIN_LOGIN]);
    file.println(String(KEY_ADMIN_PASS) + "=" + mainConfig[KEY_ADMIN_PASS]);
    file.println(String(KEY_AP_SSID) + "=" + mainConfig[KEY_AP_SSID]);
    file.println(String(KEY_AP_PASS) + "=" + mainConfig[KEY_AP_PASS]);
    file.close();
    Serial.println(INFO_UPDATE);
    LittleFS.end();
    ESP.restart();
}
