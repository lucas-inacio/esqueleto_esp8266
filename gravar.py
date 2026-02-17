import argparse
import json
import os
import platform
import shlex
import subprocess
from io import StringIO
from os import path

# 0xf0000 -> 1MB-64kB para firmware e 64kB para SPIFFS
MKFS_TARGET = "mklittlefs"
ADDRESS_SPIFFS = "0xeb000" # Encontrado em boards.txt
SKETCH_NAME = "esp8266"
TMP_FILE_NAME = "fs.bin"
ARDUINO_CLI_COMMAND_CONFIG = "arduino-cli config dump --format json"
ARDUINO_CLI_COMMAND_COMPILE = "arduino-cli compile -b esp8266:esp8266:generic:mmu=3232,eesz=1M64,ResetMethod=nodtr_nosync {Sketch} --verbose"
#ARDUINO_CLI_COMMAND_COMPILE = "arduino-cli compile -b esp8266:esp8266:generic:mmu=4816H,eesz=4M2M {Sketch} --verbose"
ARDUINO_CLI_COMMAND_UPLOAD = "arduino-cli upload -p {Port} -b esp8266:esp8266:generic:mmu=3232,eesz=1M64,ResetMethod=nodtr_nosync {Sketch} --verbose"
MKFS_ARGS = " -c esp8266/data -p 256 -b 4096 -s 0x10000 " + TMP_FILE_NAME
ESPTOOL_COMMAND_WRITE = "{Python} {Esptool} -p {Port} -b 115200 write_flash {Address} {File}"

def run_command(command_string, show_output=False):
    proc = subprocess.Popen(
        command_string.split(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if show_output:
        for line in proc.stdout:
            print(line.decode('utf-8'), end='')
        proc.wait()
    else:
        status = ['\\', '|', '/', '-']
        count = 0
        while True:
            try:
                proc.communicate(timeout=0.1)
                print('\r', end='')
                break
            except subprocess.TimeoutExpired:
                print('\r', end='')
                print(status[count], end='')
                count = count + 1
                count = count % len(status)
    
    if proc.returncode != 0:
        print('Processo falhou com código', proc.returncode)
    else:
        return True

def get_arduino_config():
    """
    Obtém as configurações de arduino-cli e as retorna um dicionário.
    A função tenta executar o programa arduino-cli que deve estar no
    caminho do sistema.

    Retorna:
    dict: Um dicionário contendo a configuração reportada por arduino-cli config dump --format json
    """
    try:
        proc = subprocess.run(shlex.split(ARDUINO_CLI_COMMAND_CONFIG), capture_output=True)
        io = StringIO(proc.stdout.decode('utf-8'))
        return json.load(io)
    except FileNotFoundError:
        raise FileNotFoundError('arduino-cli não encontrado')

def find_tool(root_dir, target):
    """
    Retorna o caminho absoluto do executável apontado por target.

    Parâmetros:
    root_dir (string): O diretório base por onde começar a procurar.

    Retorna:
    string: O caminho absoluto do executável apontado por target.
    """
    target_path = path.abspath(path.normpath(root_dir))
    if path.isdir(target_path):
        with os.scandir(target_path) as it:
            for sub in it:
                result = find_tool(sub.path, target)
                if result:
                    return result
    elif path.isfile(target_path) and target_path.split(os.sep)[-1] == target:
        return target_path

def run_mkfs(mkfs_path):
    """
    Executa o programa mklittlefs/mkspiffs como: 
    
    mklittlefs/mkspiffs -c esp8266/data -p 256 -b 8192 -s 65536 fs.out
    """
    try:
        run_command(mkfs_path + MKFS_ARGS, True)
    except subprocess.CalledProcessError:
        raise Exception('Erro ao criar sistema de arquivos com mklittlefs.')

def clean_up_temp_files():
    if path.isfile(TMP_FILE_NAME):
        print(f'Removendo {TMP_FILE_NAME}')
        os.remove(TMP_FILE_NAME)

def build_commands():
    parser = argparse.ArgumentParser(description='Manage build, data and upload')
    subparsers = parser.add_subparsers(dest='cmd')

    build_parser = subparsers.add_parser('build', help='build program')
    build_parser.add_argument('-v', '--verbose', help='show verbose output', action='store_true')

    upload_parser = subparsers.add_parser('upload', help='upload the program')
    upload_parser.add_argument('port', help='port name')

    config_parser = subparsers.add_parser('config', help='generate and upload config data to board')
    config_parser.add_argument('port', help='port name name')

    return parser

def main():
    parser = build_commands()
    args = parser.parse_args()
    if args.cmd == 'build':
        print('Compilando sketch...')
        if run_command(ARDUINO_CLI_COMMAND_COMPILE.format(Sketch=SKETCH_NAME), args.verbose):
            print('Sucesso')
    elif args.cmd == 'upload':
        print('Gravando sketch...')
        run_command(ARDUINO_CLI_COMMAND_UPLOAD.format(Port=args.port, Sketch=SKETCH_NAME), True)
    elif args.cmd == 'config':
        # Obtém diretório data do arduino-cli
        print('Obtendo configurações de arduino-cli...')
        config = get_arduino_config()

        # Encontra o executável de mklittlefs/mkspiffs a partir do diretório data
        # e cria sistema de arquivos
        root_dir = ''
        try:
            root_dir = config['directories']['data']
        except KeyError:
            home = os.path.expanduser('~')
            plat = platform.system()
            if platform.system() == 'Linux':
                root_dir = path.join(home, '.arduino15')
            elif plat == 'Windows':
                root_dir = path.join(home, 'AppData/Local/Arduino15')
            elif plat == 'Darwin':
                root_dir = path.join(home, 'Library/Arduino15')
            else:
                raise RuntimeError('os não suportado')
        
        mkfs_path = find_tool(
            path.join(root_dir, f'packages/esp8266/tools/{MKFS_TARGET}'),
            MKFS_TARGET
        )
        if not mkfs_path:
            raise FileNotFoundError(f'{MKFS_TARGET} não encontrado')
        print('Econtrado mkspiffs em', mkfs_path)
        print('Criando sistema de arquivos...')
        run_mkfs(mkfs_path)

        # Usa esptool para gravar o arquivo gerado por mkspiffs
        # no microcontrolador
        python_path = find_tool(
            path.join(root_dir, 'packages/esp8266/tools/python3'),
            'python3'
        )
        if not python_path:
            raise FileNotFoundError('python3 não encontrado')
        print('Encontrado python em', python_path)
        
        esptool_path = find_tool(
            path.join(root_dir, 'packages/esp8266/hardware'),
            'esptool.py'
        )
        if not esptool_path:
            raise FileNotFoundError('esptool não encontrado')
        print('Encontrado esptool em', esptool_path)
        print('Gravando sistema de arquivos spiffs...')

        run_command(
            ESPTOOL_COMMAND_WRITE.format(
                Python=python_path, Esptool=esptool_path, Port=args.port,
                Address=ADDRESS_SPIFFS, File=TMP_FILE_NAME
            ),
            True
        )
    else:
        parser.print_help()

if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as e:
        print('Erro ao tentar executar arduino-cli. Código', e.returncode)
        print(e)
    except FileNotFoundError as e:
        print(e)
    except Exception as e:
        print(e)
    finally:
        clean_up_temp_files()
