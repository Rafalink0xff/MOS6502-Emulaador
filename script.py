from pathlib import Path

# Pasta raiz do projeto
RAIZ = Path(__file__).resolve().parent
SRC = RAIZ / "src"

# Arquivo de saída
ARQUIVO_SAIDA = RAIZ / "codigo_projeto1.txt"

# Extensões desejadas
EXTENSOES = {".cpp", ".h"}

arquivos = []

# Adiciona o CMakeLists.txt da raiz
cmake = RAIZ / "CMakeLists.txt"
if cmake.is_file():
    arquivos.append(cmake)

# Adiciona todos os .cpp e .h da pasta src
if SRC.is_dir():
    arquivos.extend(
        sorted(
            arquivo
            for arquivo in SRC.rglob("*")
            if arquivo.is_file()
            and arquivo.suffix.lower() in EXTENSOES
        )
    )

with ARQUIVO_SAIDA.open("w", encoding="utf-8") as saida:
    if not arquivos:
        saida.write("Nenhum arquivo encontrado.\n")
    else:
        for arquivo in arquivos:
            saida.write("=" * 80 + "\n")
            saida.write(f"Arquivo: {arquivo.relative_to(RAIZ)}\n")
            saida.write("=" * 80 + "\n\n")

            try:
                conteudo = arquivo.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                conteudo = arquivo.read_text(encoding="latin-1")

            saida.write(conteudo)
            saida.write("\n\n")

print(f"Arquivo '{ARQUIVO_SAIDA.name}' criado com sucesso!")
