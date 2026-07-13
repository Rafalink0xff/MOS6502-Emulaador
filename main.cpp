#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "cpu.h" 

bool CarregarROM(CPU& nes_cpu, const std::string& caminhoArquivo) {
    std::ifstream arquivo(caminhoArquivo, std::ios::binary);

    if (!arquivo.is_open()) {
        std::cout << "ERRO FATAL: Nao foi possivel abrir a ROM: " << caminhoArquivo << "\n";
        return false;
    }

    std::vector<uint8_t> cabecalho(16);
    arquivo.read(reinterpret_cast<char*>(cabecalho.data()), 16);

    if (cabecalho[0] != 'N' || cabecalho[1] != 'E' || cabecalho[2] != 'S' || cabecalho[3] != 0x1A) {
        std::cout << "ERRO FATAL: Arquivo corrompido ou formato invalido.\n";
        return false;
    }

    std::cout << "[Cartucho] Assinatura NES detectada com sucesso!\n";

    uint8_t prgRomBancos = cabecalho[4];
    uint16_t tamanhoPRG = prgRomBancos * 16384;

    std::cout << "[Cartucho] Tamanho do Codigo (PRG-ROM): " << (tamanhoPRG / 1024) << " KB\n";

    if (cabecalho[6] & 0x04) {
        arquivo.seekg(512, std::ios::cur);
    }

    arquivo.read(reinterpret_cast<char*>(&nes_cpu.memoria[0x8000]), tamanhoPRG);
    arquivo.close();
    return true;
}

int main() {
    std::cout << "--- INICIANDO O EMULADOR NES ---\n";

    CPU nes_cpu;

    if (!CarregarROM(nes_cpu, "mario.nes")) {
        std::cin.get();
        return 1;
    }

    std::cout << "\nApertando o Reset do console...\n";
    nes_cpu.Reset();

    std::cout << "\n--- EXECUTANDO O JOGO REAL ---\n";

    // Vamos executar os 15 primeiros ciclos reais do Super Mario!
    for (int i = 0; i < 15; i++) {
        // 1. Clonamos o PC atual só para o Raio-X poder ler a instrução sem estragar o PC real
        uint16_t pc_para_leitura = nes_cpu.PC;

        // 2. Imprimimos o que vai ser executado
        std::cout << nes_cpu.DisassembleInstrucao(pc_para_leitura) << "\n";

        // 3. Deixamos a placa-mãe rodar a instrução!
        nes_cpu.ExecutarCiclo();
    }

    std::cout << "\n--- CPU PAUSADA ---\n";
    std::cin.get();
    return 0;
}