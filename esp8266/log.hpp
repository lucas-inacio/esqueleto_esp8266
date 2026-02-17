#ifndef LOG_HPP
#define LOG_HPP

#include <Arduino.h>
#include <FS.h>

#include "util.hpp"

#define LOG_FILE        F("/log.txt")
#define DATA_FILE       F("/data.txt")
#define TIMESTAMP_FILE  F("/timestamp.txt")
#define LOG_SIZE_LIMIT  32768
#define DATA_SIZE_LIMIT 1572864 // 1,5MB

// Mensagens de erro reportadas no arquivo "/log.txt"
#define ERRO_UMIDADE     F("Erro ao configurar umidade")
#define ERRO_TEMPERATURA F("Erro ao configurar temperatura")
#define ERRO_WIFI        F("Falha ao conectar. Iniciando ponto de acesso")
#define ERRO_AP          F("Falha ao inicar ponto de acesso. Reiniciando...")
#define ERRO_SENSORES    F("Falha ao ler sensores")
#define ERRO_LOG         F("Erro ao manipular arquivo de log")
#define ERRO_CONFIG      F("Falha ao carregar estado. Reiniciando...")
#define INFO_UPDATE      F("Estado modificado. Reiniciando...")
#define INFO_INI         F("Inicializando")
#define INFO_PRONTO      F("Estado carregado")
#define INFO_CHEIO       F("Arquivo de dados cheio")

template <class T>
void logFile(const T str, String currentTime) {
    File file = SPIFFS.open(LOG_FILE, "a");
    if(!file) {
        Serial.println(ERRO_LOG);
        return;
    }

    // Limita o tamanho do log a LOG_SIZE_LIMIT
    size_t size = customStringLen(str) + file.size();
    if(size >= LOG_SIZE_LIMIT) {
        file.seek(0, SeekSet);
    }

    file.print(str);
    file.print(" ");
    file.println(currentTime);
    file.close();
}

template <class T>
void registraEmArquivo(
    const T nome, float temperatura, float umidade, const String timestamp) {

    if(tamanhoDoArquivo(nome) >= DATA_SIZE_LIMIT) {
        Serial.println(INFO_CHEIO);
        // SPIFFS.remove(nome);
        return;
    }

    File arquivo = SPIFFS.open(nome, "a");
    if(!arquivo) {
        Serial.println(ERRO_LOG);
        return;
    }

    arquivo.printf("%.2f, %.2f, %s\n", temperatura, umidade, timestamp.c_str());
    arquivo.close();
}

template <class T>
unsigned long tamanhoDoArquivo(const T nome) {
    File arquivo = SPIFFS.open(nome, "r");
    if(arquivo) {
        unsigned long tamanho = arquivo.size();
        arquivo.close();
        return tamanho;
    }
    return 0;
}

#endif // LOG_HPP