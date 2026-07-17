#include "cpu.h"
#include <iostream>
#include <cstdlib>

CPU::CPU(Bus* b) {
    bus = b;
    vblank_ativo = false;
}

void CPU::Reset() {
    A = 0; X = 0; Y = 0; SP = 0xFD; Status = 0x24;

    uint8_t low = bus->Ler(0xFFFC);
    uint8_t high = bus->Ler(0xFFFD);
    PC = (high << 8) | low;
}

void CPU::NMI() {
    static int contador_frames = 0;
    contador_frames++;
    if (contador_frames % 60 == 0) {
        std::cout << "[Sistema] 60 frames processados! (1 seg. de jogo real)\n";
    }
    Push((PC >> 8) & 0xFF);
    Push(PC & 0xFF);
    Push(Status | 0x20);
    Status |= 0x04;

    uint8_t low = LerMemoria(0xFFFA);
    uint8_t high = LerMemoria(0xFFFB);
    PC = (high << 8) | low;
}

void CPU::GatilhoVBlank() {
    vblank_ativo = true;
    if (bus->Ler(0x2000) & 0x80) {
        NMI();
    }
}

int CPU::ExecutarCiclo() {
    // uint16_t pc_atual = PC;
    // std::cout << DisassembleInstrucao(pc_atual) << "\n";

    uint8_t opcode = LerMemoria(PC);
    PC++;

    switch (opcode) {
        case 0xA9: Instrucao_LDA(ModoImediato());  break;
        case 0xA5: Instrucao_LDA(ModoZeroPage());  break;
        case 0xA6: Instrucao_LDX(ModoZeroPage());  break;
        case 0xAD: Instrucao_LDA(ModoAbsoluto()); break;
        case 0xBD: Instrucao_LDA(ModoAbsolutoX()); break;
        case 0xB9: Instrucao_LDA(ModoAbsolutoY()); break;
        case 0xB1: Instrucao_LDA(ModoIndiretoY()); break;
        case 0xB5: Instrucao_LDA(ModoZeroPageX()); break;
        case 0xA0: Instrucao_LDY(ModoImediato());  break;
        case 0xA4: Instrucao_LDY(ModoZeroPage());  break;
        case 0xAC: Instrucao_LDY(ModoAbsoluto()); break;
        case 0xBC: Instrucao_LDY(ModoAbsolutoX()); break;
        case 0x8D: Instrucao_STA(ModoAbsoluto());  break;
        case 0x95: Instrucao_STA(ModoZeroPageX()); break;
        case 0x8E: Instrucao_STX(ModoAbsoluto());  break;
        case 0x85: Instrucao_STA(ModoZeroPage());  break;
        case 0x86: Instrucao_STX(ModoZeroPage());  break;
        case 0x8C: Instrucao_STY(ModoAbsoluto());  break;
        case 0x84: Instrucao_STY(ModoZeroPage());  break;
        case 0x91: Instrucao_STA(ModoIndiretoY()); break;
        case 0x99: Instrucao_STA(ModoAbsolutoY()); break;
        case 0x9D: Instrucao_STA(ModoAbsolutoX()); break;

        case 0xAA: Instrucao_TAX(); break;
        case 0x8A: Instrucao_TXA(); break;
        case 0xE8:
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
        case 0xC5: Instrucao_CMP(ModoZeroPage()); break;
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
        case 0x0E: Instrucao_ASL(ModoAbsoluto()); break;
        case 0x2A: Instrucao_ROL_Acumulador();    break;
        case 0x6A: Instrucao_ROR_A(); break;
        case 0x26: Instrucao_ROL(ModoZeroPage()); break;
        case 0x2E: Instrucao_ROL(ModoAbsoluto()); break;
        case 0x7E: Instrucao_ROR(ModoAbsolutoX()); break;
        case 0xF9: Instrucao_SBC(ModoAbsolutoY()); break;
        case 0xED: Instrucao_SBC(ModoAbsoluto()); break;
        case 0xE9: Instrucao_SBC(ModoImediato());  break;
        case 0xF5: Instrucao_SBC(ModoZeroPageX()); break;
        case 0x6D: Instrucao_ADC(ModoAbsoluto());  break;
        case 0x7D: Instrucao_ADC(ModoAbsolutoX()); break;
        case 0xD5: Instrucao_CMP(ModoZeroPageX()); break;
        case 0xDD: Instrucao_CMP(ModoAbsolutoX()); break;
        case 0x79: Instrucao_ADC(ModoAbsolutoY()); break;
        case 0x65: Instrucao_ADC(ModoZeroPage());  break;
        case 0x75: Instrucao_ADC(ModoZeroPageX()); break;
        case 0x69: Instrucao_ADC(ModoImediato());  break;
        case 0xE6: Instrucao_INC(ModoZeroPage()); break;

        case 0x10: Instrucao_BPL(ModoRelativo()); break;
        case 0x30: Instrucao_BMI(ModoRelativo()); break;
        case 0xB0: Instrucao_BCS(ModoRelativo()); break;
        case 0x90: Instrucao_BCC(ModoRelativo()); break;
        case 0xD0: Instrucao_BNE(ModoRelativo()); break;
        case 0xF0: Instrucao_BEQ(ModoRelativo()); break;
        case 0x09: Instrucao_ORA(ModoImediato()); break;
        case 0x0D: Instrucao_ORA(ModoAbsoluto()); break;
        case 0x05: Instrucao_ORA(ModoZeroPage()); break;
        case 0x29: Instrucao_AND(ModoImediato()); break;
        case 0x2D: Instrucao_AND(ModoAbsoluto()); break;
        case 0x3D: Instrucao_AND(ModoAbsolutoX()); break;
        case 0x39: Instrucao_AND(ModoAbsolutoY()); break;
        case 0x25: Instrucao_AND(ModoZeroPage()); break;
        case 0x45: Instrucao_EOR(ModoZeroPage()); break;
        case 0x49: Instrucao_EOR(ModoImediato());  break;

        case 0x9A: SP = X; break;
        case 0x78: Status |= 0x04; break;
        case 0xD8: Status &= ~0x08; break;
        case 0x48: Instrucao_PHA(); break;
        case 0x68: Instrucao_PLA(); break;
        case 0x38: Instrucao_SEC(); break;
        case 0x18: Instrucao_CLC(); break;

        case 0x20: Instrucao_JSR(ModoAbsoluto()); break;
        case 0x60: Instrucao_RTS(); break;
        case 0x40: Instrucao_RTI(); break;
        case 0x4C: Instrucao_JMP(ModoAbsoluto()); break;
        case 0x6C: Instrucao_JMP(ModoIndireto()); break;

        case 0x00:
            std::cout << "Interrupcao forcada (BRK).\n";
            exit(0);

        default:
            std::cout << "\nERRO FATAL: Instrucao desconhecida lida pela CPU! Opcode: 0x"
                      << std::uppercase << std::hex << (int)opcode
                      << " no PC: 0x" << PC-1 << "\n";
            std::cout << "Pressione ENTER para sair...\n";
            std::cin.get();
            exit(1);
    }

    static const uint8_t CICLOS_6502[256] = {
        7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
        2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
        2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
        2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
        2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
        2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
        2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7
    };

    return CICLOS_6502[opcode];
}

uint8_t CPU::LerMemoria(uint16_t endereco) {
    // A CPU não intercepta mais NADA. Ela só confia no barramento.
    return bus->Ler(endereco);
}

void CPU::EscreverMemoria(uint16_t endereco, uint8_t valor) {
    bus->Escrever(endereco, valor);
}

void CPU::Push(uint8_t valor) {
    EscreverMemoria(0x0100 + SP, valor);
    SP--;
}

uint8_t CPU::Pop() {
    SP++;
    return LerMemoria(0x0100 + SP);
}