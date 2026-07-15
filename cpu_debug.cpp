#include "cpu.h"
#include <sstream>
#include <iomanip>

std::string CPU::DisassembleInstrucao(uint16_t& endereco) {
    uint8_t opcode = memoria[endereco];
    std::stringstream ss;

    ss << "[" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << endereco << "] ";

    switch (opcode) {
        // --- 1 Byte ---
        case 0xAA: ss << "AA       TAX"; endereco += 1; break;
        case 0x8A: ss << "8A       TXA"; endereco += 1; break;
        case 0x9A: ss << "9A       TXS"; endereco += 1; break;
        case 0xE8: ss << "E8       INX"; endereco += 1; break;
        case 0xCA: ss << "CA       DEX"; endereco += 1; break;
        case 0xC8: ss << "C8       INY"; endereco += 1; break;
        case 0x88: ss << "88       DEY"; endereco += 1; break;
        case 0x78: ss << "78       SEI"; endereco += 1; break;
        case 0xD8: ss << "D8       CLD"; endereco += 1; break;
        case 0x60: ss << "60       RTS"; endereco += 1; break;

        // --- 2 Bytes ---
        case 0xB1:
        {
            uint8_t ponteiro_zp = memoria[endereco + 1];
            ss << "B1 " << std::setfill('0') << std::setw(2) << (int)ponteiro_zp
               << "    LDA ($" << std::setw(2) << (int)ponteiro_zp << "),Y";
            endereco += 2; break;
        }
        case 0xA9:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "A9 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDA #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xA0:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "A0 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDY #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xA2:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "A2 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDX #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xC9:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "C9 " << std::setfill('0') << std::setw(2) << (int)valor << "    CMP #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xE0:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "E0 " << std::setfill('0') << std::setw(2) << (int)valor << "    CPX #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xC0:
        {
            uint8_t valor = memoria[endereco + 1];
            ss << "C0 " << std::setfill('0') << std::setw(2) << (int)valor << "    CPY #$" << (int)valor;
            endereco += 2; break;
        }
        case 0x29:
            ss << "29 " << std::setfill('0') << std::setw(2) << (int)memoria[endereco + 1]
               << "       AND #$" << std::setw(2) << (int)memoria[endereco + 1];
            endereco += 2; break;
        case 0x10:
        {
            int8_t deslocamento = (int8_t)memoria[endereco + 1];
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "10 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BPL $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0xB0:
        {
            int8_t deslocamento = (int8_t)memoria[endereco + 1];
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "B0 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BCS $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0xD0:
        {
            int8_t deslocamento = (int8_t)memoria[endereco + 1];
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "D0 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BNE $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0x85:
        {
            uint8_t endereco_zp = memoria[endereco + 1];
            ss << "85 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "    STA $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0x86:
        {
            uint8_t endereco_zp = memoria[endereco + 1];
            ss << "86 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "    STX $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0x91:
        {
            uint8_t ponteiro_zp = memoria[endereco + 1];
            ss << "91 " << std::setfill('0') << std::setw(2) << (int)ponteiro_zp
               << "    STA ($" << std::setw(2) << (int)ponteiro_zp << "),Y";
            endereco += 2; break;
        }
        case 0x09:
            ss << "09 " << std::setfill('0') << std::setw(2) << (int)memoria[endereco + 1]
               << "       ORA #$" << std::setw(2) << (int)memoria[endereco + 1];
            endereco += 2; break;

        // --- 3 Bytes ---
        case 0x9D:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "9D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0xBE:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "BE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDX $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0xAE:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "AE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDX $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xAC:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "AC " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDY $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xEE:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "EE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    INC $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x4C:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "4C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    JMP $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xAD:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "AD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDA $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xBD:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "BD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x8D:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "8D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STA $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x20:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "20 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    JSR $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x2C:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "2C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    BIT $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x99:
        {
            uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
            ss << "99 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }

        default:
            ss << std::setfill('0') << std::setw(2) << std::hex << (int)opcode << "       ??? (Nao Implementado)";
            endereco += 1;
            break;
    }
    return ss.str();
}