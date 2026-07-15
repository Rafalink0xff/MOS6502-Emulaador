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
        case 0xA5: Instrucao_LDA(ModoZeroPage());  break;
        case 0xA6: Instrucao_LDX(ModoZeroPage());  break;
        case 0xAD: Instrucao_LDA(ModoAbsoluto());  break;
        case 0xBD: Instrucao_LDA(ModoAbsolutoX()); break;
        case 0xB9: Instrucao_LDA(ModoAbsolutoY()); break;
        case 0xB1: Instrucao_LDA(ModoIndiretoY()); break;
        case 0xA0: Instrucao_LDY(ModoImediato());  break;
        case 0xA4: Instrucao_LDY(ModoZeroPage());  break;
        case 0xAC: Instrucao_LDY(ModoAbsoluto()); break;
        case 0xBC: Instrucao_LDY(ModoAbsolutoX()); break;
        case 0x8D: Instrucao_STA(ModoAbsoluto());  break;
        case 0x8E: Instrucao_STX(ModoAbsoluto());  break;
        case 0x85: Instrucao_STA(ModoZeroPage());  break;
        case 0x86: Instrucao_STX(ModoZeroPage());  break;
        case 0x8C: Instrucao_STY(ModoAbsoluto());  break;
        case 0x84: Instrucao_STY(ModoZeroPage());  break;
        case 0x91: Instrucao_STA(ModoIndiretoY()); break;
        case 0x99: Instrucao_STA(ModoAbsolutoY()); break;
        case 0x9D: Instrucao_STA(ModoAbsolutoX()); break;

        // ==========================================
        // 2. INSTRUÇÕES DE REGISTRADORES E MATEMÁTICA
        // ==========================================
        case 0xAA: Instrucao_TAX(); break;
        case 0x8A: Instrucao_TXA(); break;
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
        case 0xCD: Instrucao_CMP(ModoAbsoluto()); break;
        case 0xD9: Instrucao_CMP(ModoAbsolutoY()); break;
        case 0xE0: Instrucao_CPX(ModoImediato()); break;
        case 0xA8: Instrucao_TAY(); break;
        case 0x98: Instrucao_TYA(); break;
        case 0xC0: Instrucao_CPY(ModoImediato()); break;
        case 0x2C: Instrucao_BIT(ModoAbsoluto()); break;
        case 0x24: Instrucao_BIT(ModoZeroPage()); break;
        case 0xEE: Instrucao_INC(ModoAbsoluto()); break;
        case 0xCE: Instrucao_DEC(ModoAbsoluto()); break;
        case 0xC6: Instrucao_DEC(ModoZeroPage()); break;
        case 0xDE: Instrucao_DEC(ModoAbsolutoX()); break;
        case 0x4A: Instrucao_LSR_Acumulador();    break;
        case 0x46: Instrucao_LSR(ModoZeroPage()); break;
        case 0x4E: Instrucao_LSR(ModoAbsoluto()); break;
        case 0x0A: Instrucao_ASL_Acumulador();    break;
        case 0x2A: Instrucao_ROL_Acumulador();    break;
        case 0x26: Instrucao_ROL(ModoZeroPage()); break;
        case 0x7E: Instrucao_ROR(ModoAbsolutoX()); break;
        case 0xF9: Instrucao_SBC(ModoAbsolutoY()); break;
        case 0xE9: Instrucao_SBC(ModoImediato());  break;
        case 0x6D: Instrucao_ADC(ModoAbsoluto());  break;
        case 0x79: Instrucao_ADC(ModoAbsolutoY()); break;
        case 0x65: Instrucao_ADC(ModoZeroPage());  break;
        case 0x69: Instrucao_ADC(ModoImediato());  break;
        case 0xE6: Instrucao_INC(ModoZeroPage()); break;

        // ==========================================
        // 3. DESVIOS CONDICIONAIS (BRANCHES)
        // ==========================================
        case 0x10: Instrucao_BPL(ModoRelativo()); break;
        case 0x30: Instrucao_BMI(ModoRelativo()); break;
        case 0xB0: Instrucao_BCS(ModoRelativo()); break;
        case 0x90: Instrucao_BCC(ModoRelativo()); break;
        case 0xD0: Instrucao_BNE(ModoRelativo()); break;
        case 0xF0: Instrucao_BEQ(ModoRelativo()); break;
        case 0x09: Instrucao_ORA(ModoImediato()); break;
        case 0x05: Instrucao_ORA(ModoZeroPage()); break;
        case 0x29: Instrucao_AND(ModoImediato()); break;
        case 0x3D: Instrucao_AND(ModoAbsolutoX()); break;
        case 0x45: Instrucao_EOR(ModoZeroPage()); break;
        case 0x49: Instrucao_EOR(ModoImediato());  break;

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
        case 0x48: Instrucao_PHA(); break;
        case 0x68: Instrucao_PLA(); break;
        case 0x38: Instrucao_SEC(); break;
        case 0x18: Instrucao_CLC(); break;

        // ==========================================
        // 5. CHAMADAS DE FUNÇÃO E SALTOS
        // ==========================================
        case 0x20: Instrucao_JSR(ModoAbsoluto()); break;
        case 0x60: Instrucao_RTS(); break;
        case 0x40: Instrucao_RTI(); break;
        case 0x4C: Instrucao_JMP(ModoAbsoluto()); break;
        case 0x6C: Instrucao_JMP(ModoIndireto()); break;

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