from pathlib import Path

# Pasta onde o script está (raiz do projeto)
RAIZ = Path(__file__).resolve().parent

# Arquivo de saída
ARQUIVO_SAIDA = RAIZ / "codigo_projeto1.txt"

# Extensões desejadas
EXTENSOES = {".cpp", ".h"}

# Nome de arquivos extras que também devem ser incluídos
ARQUIVOS_EXTRAS = {"CMakeLists.txt"}

# Apenas arquivos da pasta raiz (não percorre subpastas)
arquivos = sorted(
    arquivo for arquivo in RAIZ.iterdir()
    if arquivo.is_file()
    and (
            arquivo.suffix.lower() in EXTENSOES
            or arquivo.name in ARQUIVOS_EXTRAS
    )
)

with ARQUIVO_SAIDA.open("w", encoding="utf-8") as saida:
    if not arquivos:
        saida.write("Nenhum arquivo .cpp, .h ou CMakeLists.txt encontrado na pasta raiz.\n")
    else:
        for arquivo in arquivos:
            saida.write("=" * 80 + "\n")
            saida.write(f"Arquivo: {arquivo.name}\n")
            saida.write(f"Caminho relativo: {arquivo.relative_to(RAIZ)}\n")
            saida.write(f"Caminho completo: {arquivo.resolve()}\n")
            saida.write("=" * 80 + "\n\n")

            try:
                conteudo = arquivo.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                conteudo = arquivo.read_text(encoding="latin-1")

            saida.write(conteudo)
            saida.write("\n\n")

print(f"Arquivo '{ARQUIVO_SAIDA.name}' criado com sucesso!")