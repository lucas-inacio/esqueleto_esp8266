# Esqueleto de projeto para esp8266
Inclui servidor web para permitir configuração de credenciais Wi-Fi

## Ferramentas necessárias
O projeto usa como base a plataforma ESP-01 (ESP8266). As seguintes dependências são necessárias:
- arduino-cli
- python 3

## Instalando dependências
É necessário instalar as ferramentas de compilação do ESP8266 para o Arduino e as bibliotecas.
```shell
arduino-cli config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli update
arduino-cli core install esp8266:esp8266
arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncTCP.git
arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncWebServer.git
```

## Compilando o projeto
O projeto utiliza um layout de memória específico. As etapas abaixo devem ser seguidas para configurar o projeto corretamente.

### Gerando os arquivos web
O diretório web contém as páginas do servidor. É necessário gerar um arquivo de cabeçalho para gravar as páginas no ESP8266.
```shell
python compress_data.py -c ./esp8266/site_data.h ./web
```

### Arquivo de configuração
Dentro do diretório esp8266 há um modelo de arquivo de configuração. Nele devem constar informações como as credenciais do Wi-Fi, etc. Crie uma cópia deste arquivo e renomeie para esp8266/data/config.txt. Então grave o novo sistema de arquivos como segue:
```shell
python gravar.py config <Porta COM>
```

### Compilação
Digite:
```shell
python gravar.py build
```

### Gravação
Digite:
```shell
pyhton gravar.py upload <Porta COM>
```
