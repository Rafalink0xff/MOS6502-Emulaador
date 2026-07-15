#include "cpu.h"
#include <iostream>
#include <cstdlib>

CPU::CPU() {
    for (int i = 0; i < 65536; i++) {
        memoria[i] = 0;
    }
}

void CPU::Reset() {
    A = 0;
    X = 0;
    Y = 0;
    SP = 0xFD;
    Status = 0x24;

    uint8_t low = memoria[0xFFFC];
    uint8_t high = memoria[0xFFFD];
    PC = (high << 8) | low;
}

void CPU::NMI() {
    // 1. Salva o PC atual na pilha (Alto e depois Baixo)
    Push((PC >> 8) & 0xFF);
    Push(PC & 0xFF);

    // 2. Salva o Status na pilha (o NES sempre força o bit 5 como '1' ao salvar)
    Push(Status | 0x20);

    // 3. Liga a flag de Interrupção para evitar que outro choque atropele esse
    Status |= 0x04;

    // 4. Lê para onde devemos pular (Vetor NMI gravado no final do cartucho)
    uint8_t low = LerMemoria(0xFFFA);
    uint8_t high = LerMemoria(0xFFFB);
    PC = (high << 8) | low;
}

void CPU::ExecutarCiclo() {
    // DIAGNÓSTICO: Descomente a linha abaixo para ver o log, mas mantenha comentado
    // se o loop infinito (JMP Absoluto) for poluir demais sua tela esperando o NMI.

    // uint16_t pc_atual = PC;
    // std::cout << DisassembleInstrucao(pc_atual) << "\n";

    uint8_t opcode = LerMemoria(PC);
    PC++;

    switch (opcode) {

        // ==========================================
        // 1. INSTRUÇÕES DE MEMÓRIA (LOAD/STORE)
        // ==========================================
        case 0xA9: Instrucao_LDA(ModoImediato());  break;
        case 0xAD: Instrucao_LDA(ModoAbsoluto());  break;
        case 0xBD: Instrucao_LDA(ModoAbsolutoX()); break;
        case 0xB1: Instrucao_LDA(ModoIndiretoY()); break;
        case 0xA0: Instrucao_LDY(ModoImediato());  break;
        case 0xAC: Instrucao_LDY(ModoAbsoluto()); break;
        case 0x8D: Instrucao_STA(ModoAbsoluto());  break;
        case 0x85: Instrucao_STA(ModoZeroPage());  break;
        case 0x86: Instrucao_STX(ModoZeroPage());  break;
        case 0x91: Instrucao_STA(ModoIndiretoY()); break;
        case 0x99: Instrucao_STA(ModoAbsolutoY()); break;
        case 0x9D: Instrucao_STA(ModoAbsolutoX()); break;

        // ==========================================
        // 2. INSTRUÇÕES DE REGISTRADORES E MATEMÁTICA
        // ==========================================
        case 0xAA: // TAX
            X = A;
            AtualizarFlagsZeroENegativo(X);
            break;
        case 0x8A: // TXA
            A = X;
            AtualizarFlagsZeroENegativo(A);
            break;
        case 0xE8: // INX
            X = X + 1;
            AtualizarFlagsZeroENegativo(X);
            break;
        case 0xA2: Instrucao_LDX(ModoImediato());  break;
        case 0xAE: Instrucao_LDX(ModoAbsoluto());  break;
        case 0xBE: Instrucao_LDX(ModoAbsolutoY()); break;
        case 0xCA: Instrucao_DEX(); break;
        case 0xC8: Instrucao_INY(0); break;
        case 0x88: Instrucao_DEY(); break;
        case 0xC9: Instrucao_CMP(ModoImediato()); break;
        case 0xE0: Instrucao_CPX(ModoImediato()); break;
        case 0xC0: Instrucao_CPY(ModoImediato()); break;
        case 0x2C: Instrucao_BIT(ModoAbsoluto()); break;
        case 0xEE: Instrucao_INC(ModoAbsoluto()); break;

        // ==========================================
        // 3. DESVIOS CONDICIONAIS (BRANCHES)
        // ==========================================
        case 0x10: Instrucao_BPL(ModoRelativo()); break;
        case 0xB0: Instrucao_BCS(ModoRelativo()); break;
        case 0xD0: Instrucao_BNE(ModoRelativo()); break;
        case 0x09: Instrucao_ORA(ModoImediato()); break;
        case 0x29: Instrucao_AND(ModoImediato()); break;

        // ==========================================
        // 4. PILHA E STATUS (FLAGS)
        // ==========================================
        case 0x9A:
            SP = X;
            break;
        case 0x78:
            Status |= 0x04;
            break;
        case 0xD8:
            Status &= ~0x08;
            break;

        // ==========================================
        // 5. CHAMADAS DE FUNÇÃO E SALTOS
        // ==========================================
        case 0x20: Instrucao_JSR(ModoAbsoluto()); break;
        case 0x60: Instrucao_RTS(); break;
        case 0x4C: Instrucao_JMP(ModoAbsoluto()); break;

        // ==========================================
        // 6. SISTEMA
        // ==========================================
        case 0x00:
            std::cout << "Interrupcao forcada (BRK).\n";
            exit(0);

        default:
            std::cout << "ERRO FATAL: Instrucao desconhecida lida pela CPU! Opcode: 0x"
                      << std::uppercase << std::hex << (int)opcode << "\n";
            exit(1);
    }
}

// ==========================================
// BARRAMENTO DE MEMÓRIA E PILHA
// ==========================================

uint8_t CPU::LerMemoria(uint16_t endereco) {
    if (endereco == 0x2002) {
        return 0x80; // HACK DO VBLANK
    }
    return memoria[endereco];
}

void CPU::EscreverMemoria(uint16_t endereco, uint8_t valor) {
    memoria[endereco] = valor;
}

void CPU::Push(uint8_t valor) {
    EscreverMemoria(0x0100 + SP, valor);
    SP--;
}

uint8_t CPU::Pop() {
    SP++;
    return LerMemoria(0x0100 + SP);
}