#include "cpu.h"
#include <iostream>
#include <fstream>

int main() {
    CPU minhaCpu;

    // ==============================================
    // 1. INSERINDO O CARTUCHO (Carga da ROM)
    // ==============================================
    std::ifstream arquivo("mario.nes", std::ios::binary);
    if (!arquivo) {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo mario.nes!\n";
        return 1;
    }

    // Pula o cabeçalho iNES (16 bytes)
    arquivo.seekg(16, std::ios::beg);

    // Lê os 32KB do código do jogo e grava direto nos endereços 0x8000 a 0xFFFF
    arquivo.read(reinterpret_cast<char*>(&minhaCpu.memoria[0x8000]), 32768);

    std::cout << "[Cartucho] Jogo carregado na memoria com sucesso!\n";

    // ==============================================
    // 2. LIGANDO O CONSOLE
    // ==============================================
    minhaCpu.Reset();

    int ciclos = 0;
    std::cout << "--- INICIANDO O EMULADOR NES ---\n";
    std::cout << "Aguardando o NMI arrancar o Mario do loop infinito...\n\n";

    // Loop infinito do Console ligado!
    while (true) {
        // DIAGNÓSTICO: Descomente as duas linhas abaixo para ver o log voltar à vida!
        uint16_t pc_atual = minhaCpu.PC;
        std::cout << minhaCpu.DisassembleInstrucao(pc_atual) << "\n";

        minhaCpu.ExecutarCiclo();
        ciclos++;

        // A cada 30.000 ciclos, a placa de vídeo avisa que a tela atualizou
        if (ciclos >= 30000) {
            minhaCpu.NMI();
            ciclos = 0; // Reseta o reloginho
        }
    }

    return 0;
}