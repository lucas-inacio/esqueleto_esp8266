import argparse
import gzip
import pathlib
import shutil

def write_carray(data, path, file_out):
    # Gera uma lista com a representação textual dos bytes contidos no arquivo
    bytesList = data.hex(',', 1).split(',')
    textOutput = ''
    entriesPerLine = 20
    count = 0
    # Limita o número de colunas escritas no arquivo
    for item in bytesList:
        if not count % entriesPerLine:
            textOutput = textOutput + '\n    '
        textOutput = textOutput + f'0x{item:2},'
        count = count + 1

    filename = 'out_' + path.name.strip('.\\').replace('.', '_')
    filename = filename + ('' if filename.endswith('_gz') else '_gz')
    startDefinition = '\n\nconst byte ' + filename
    startDefinition = startDefinition + '[] PROGMEM = {\n'
    file_out.write(startDefinition)
    file_out.write(textOutput[:-1])
    file_out.write('\n};\n\n')
    # Retorna o nome da array baseado no nome do arquivo
    return filename

# Escreve uma estrutura do tipo GzipData mapeando os nomes aos dados gerados por write_carray
def write_cmap(data_map, file_out):
    # Definição da estrutura GzipData e da variável gzipDataMap
    textOut = ('\nstruct GzipData {\n'
            '    const char *path;\n'
            '    const byte *data;\n'
            '    const size_t dataSize;\n'
            '};\n\n'
            'const struct GzipData gzipDataMap[] = {\n'
            )
    for data in data_map:
        textOut = textOut + '    { "' + data['path'] + '", ' + data['data'] + ', ' + data['dataSize'] + ' },\n'
    file_out.write(textOut[:-2] + '\n};')
    # Número de elementos
    file_out.write('\n\nconst size_t gzipDataCount = ' + str(len(data_map)) + ';')

def compress_to_c_header(dir, filename, use_root=True):
    with open(filename, 'wt') as file_out:
        file_out.write('#include <Arduino.h>\n');
        root = pathlib.Path(dir)
        data_map = []
        # Itera, recursivamente, sobre todos os arquivos e diretórios
        for child in root.glob('**/*'):
            if child.is_file():
                compressed_data = None
                if child.suffix == '.gz':
                    with gzip.open(child, 'rb') as file_in:
                        compressed_data = gzip.compress(file_in.read())
                else:
                    with open(child, 'rb') as file_in:
                        compressed_data = gzip.compress(file_in.read())

                array_name = write_carray(compressed_data, child, file_out)
                url_path = '/' + child.relative_to(root.name).as_posix()
                if use_root:
                    url_path = '/' + root.joinpath(child.relative_to(root.name)).as_posix()
                data_map.append({ 
                    'path': url_path[1:] if url_path[1] == '/' else url_path,
                    'data': array_name,
                    'dataSize': str(len(compressed_data))
                })
        # Cria o vetor de estruturas a partir dos dados gerados no loop acima
        write_cmap(data_map, file_out)

def compress_files_in_dir(dir):
    for entry in pathlib.Path(dir).glob('**/*'):
        if entry.is_file() and entry.suffix != '.gz':
            with open(entry.absolute(), 'rb') as file_in:
                with gzip.open(str(entry.absolute()) + '.gz', 'wb') as file_out:
                    shutil.copyfileobj(file_in, file_out)

def main():
    parser = argparse.ArgumentParser(
        description='Comprime os arquivos em dir para o formato gzip')
    parser.add_argument('dir', nargs=1, help='Diretório dos arquivos a serem comprimidos')
    parser.add_argument(
        '-c',
        dest='cfile',
        nargs=1,
        help='Gera um arquivo de cabeçalho C com os dados dos arquivos comprimidos'
    )
    parser.add_argument(
        '--root', dest='root',
        help='Usa o diretório fornecido como parte da url gerada no arquivo C (padrão: falso)',
        action='store_true'
    )
    args = parser.parse_args()

    if pathlib.Path(args.dir[0]).is_dir():
        if args.cfile:
            compress_to_c_header(args.dir[0], args.cfile[0], args.root)
        else:
            compress_files_in_dir(args.dir[0])
    else:
        print('Argumento não é um diretório ou não o diretório não existe')

if __name__ == '__main__':
    main()
