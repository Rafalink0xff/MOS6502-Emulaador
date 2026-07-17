#include "cpu.h"
#include <sstream>
#include <iomanip>
#include <deque>
#include <iostream>

std::string CPU::DisassembleInstrucao(uint16_t& endereco) {
    uint8_t opcode = bus->Ler(endereco);
    std::stringstream ss;

    ss << "[" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << endereco << "] ";

    switch (opcode) {
        // --- 1 Byte ---
        case 0x6A: ss << "6A       ROR A"; endereco += 1; break;
        case 0x98: ss << "98       TYA"; endereco += 1; break;
        case 0x40: ss << "40       RTI"; endereco += 1; break;
        case 0xA8: ss << "A8       TAY"; endereco += 1; break;
        case 0x0A: ss << "0A       ASL A"; endereco += 1; break;
        case 0x18: ss << "18       CLC"; endereco += 1; break;
        case 0x38: ss << "38       SEC"; endereco += 1; break;
        case 0x2A: ss << "2A       ROL A"; endereco += 1; break;
        case 0x68: ss << "68       PLA"; endereco += 1; break;
        case 0x4A: ss << "4A       LSR A"; endereco += 1; break;
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
        case 0x48: ss << "48       PHA"; endereco += 1; break;

        // --- 2 Bytes ---
        case 0x94:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "94 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       STY $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0xE5:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "E5 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       SBC $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xB4:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "B4 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       LDY $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0x25:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "25 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       AND $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xE6:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "E6 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       INC $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xF5:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "F5 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       SBC $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0xD5:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "D5 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       CMP $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0x95:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "95 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       STA $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0xB5:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "B5 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       LDA $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0xC5:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "C5 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       CMP $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x46:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "46 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       LSR $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x26:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "26 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       ROL $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x49:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "49 " << std::setfill('0') << std::setw(2) << (int)valor
               << "    EOR #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xA4:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "A4 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       LDY $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x24:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "24 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       BIT $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x84:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "84 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       STY $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xE9:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "E9 " << std::setfill('0') << std::setw(2) << (int)valor
               << "    SBC #$" << (int)valor;
            endereco += 2; break;
        }
        case 0x30:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "30 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BMI $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0xA6:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "A6 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       LDX $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0x65:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "65 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       ADC $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xC6:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "C6 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       DEC $" << std::setw(2) << (int)zp;
            endereco += 2; break;
        }
        case 0xA5:
        {
            uint8_t endereco_zp = bus->Ler(endereco + 1);
            ss << "A5 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "       LDA $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0x69:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "69 " << std::setfill('0') << std::setw(2) << (int)valor << "    ADC #$" << (int)valor;
            endereco += 2; break;
        }
        case 0x45:
        {
            uint8_t endereco_zp = bus->Ler(endereco + 1);
            ss << "45 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "       EOR $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0xF0:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "F0 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BEQ $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0x05:
        {
            uint8_t endereco_zp = bus->Ler(endereco + 1);
            ss << "05 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "       ORA $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0xB1:
        {
            uint8_t ponteiro_zp = bus->Ler(endereco + 1);
            ss << "B1 " << std::setfill('0') << std::setw(2) << (int)ponteiro_zp
               << "    LDA ($" << std::setw(2) << (int)ponteiro_zp << "),Y";
            endereco += 2; break;
        }
        case 0xA9:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "A9 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDA #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xA0:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "A0 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDY #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xA2:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "A2 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDX #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xC9:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "C9 " << std::setfill('0') << std::setw(2) << (int)valor << "    CMP #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xE0:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "E0 " << std::setfill('0') << std::setw(2) << (int)valor << "    CPX #$" << (int)valor;
            endereco += 2; break;
        }
        case 0xC0:
        {
            uint8_t valor = bus->Ler(endereco + 1);
            ss << "C0 " << std::setfill('0') << std::setw(2) << (int)valor << "    CPY #$" << (int)valor;
            endereco += 2; break;
        }
        case 0x29:
            ss << "29 " << std::setfill('0') << std::setw(2) << (int)bus->Ler(endereco + 1)
               << "       AND #$" << std::setw(2) << (int)bus->Ler(endereco + 1);
            endereco += 2; break;
        case 0x10:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "10 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BPL $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0xB0:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "B0 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BCS $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0xD0:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "D0 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BNE $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }
        case 0x85:
        {
            uint8_t endereco_zp = bus->Ler(endereco + 1);
            ss << "85 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "    STA $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0x86:
        {
            uint8_t endereco_zp = bus->Ler(endereco + 1);
            ss << "86 " << std::setfill('0') << std::setw(2) << (int)endereco_zp
               << "    STX $" << std::setw(2) << (int)endereco_zp;
            endereco += 2; break;
        }
        case 0x91:
        {
            uint8_t ponteiro_zp = bus->Ler(endereco + 1);
            ss << "91 " << std::setfill('0') << std::setw(2) << (int)ponteiro_zp
               << "    STA ($" << std::setw(2) << (int)ponteiro_zp << "),Y";
            endereco += 2; break;
        }
        case 0x09:
            ss << "09 " << std::setfill('0') << std::setw(2) << (int)bus->Ler(endereco + 1)
               << "       ORA #$" << std::setw(2) << (int)bus->Ler(endereco + 1);
            endereco += 2; break;

        case 0x90:
        {
            int8_t deslocamento = (int8_t)bus->Ler(endereco + 1);
            uint16_t destino = endereco + 2 + deslocamento;
            ss << "90 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
               << "    BCC $" << std::setfill('0') << std::setw(4) << destino;
            endereco += 2; break;
        }

        // --- 3 Bytes ---
        case 0x1D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "1D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ORA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x75:
        {
            uint8_t zp = bus->Ler(endereco + 1);
            ss << "75 " << std::setfill('0') << std::setw(2) << (int)zp
               << "       ADC $" << std::setw(2) << (int)zp << ",X";
            endereco += 2; break;
        }
        case 0xDD:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "DD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    CMP $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x7D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "7D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ADC $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x2D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "2D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    AND $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x0E:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "0E " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ASL $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x39:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "39 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    AND $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0x0D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "0D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ORA $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x2E:
        {
            uint8_t l = bus->Ler(endereco + 1);
            uint8_t h = bus->Ler(endereco + 2);
            ss << "2E " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ROL $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x4E:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "4E " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LSR $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xDE:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "DE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    DEC $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0xBC:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "BC " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDY $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0xD9:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "D9 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    CMP $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0xCD:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "CD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    CMP $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x79:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "79 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ADC $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0x8E:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "8E " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STX $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x8C:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "8C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STY $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x6D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "6D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ADC $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xB9:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "B9 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0x6C:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "6C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    JMP ($" << std::setw(2) << (int)h << std::setw(2) << (int)l << ")";
            endereco += 3; break;
        }
        case 0x7E:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "7E " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    ROR $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0xCE:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "CE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    DEC $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xED:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "ED " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    SBC $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xF9:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "F9 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    SBC $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0x3D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "3D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    AND $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x9D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "9D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0xBE:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "BE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDX $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",Y";
            endereco += 3; break;
        }
        case 0xAE:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "AE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDX $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xAC:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "AC " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDY $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xEE:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "EE " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    INC $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x4C:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "4C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    JMP $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xAD:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "AD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDA $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0xBD:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "BD " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    LDA $" << std::setw(2) << (int)h << std::setw(2) << (int)l << ",X";
            endereco += 3; break;
        }
        case 0x8D:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "8D " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    STA $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x20:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "20 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    JSR $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x2C:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
            ss << "2C " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
               << "    BIT $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
            endereco += 3; break;
        }
        case 0x99:
        {
            uint8_t l = bus->Ler(endereco + 1); uint8_t h = bus->Ler(endereco + 2);
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

void CPU::SalvarLogCircular() {
    // Formata uma string rápida com o estado atual: "PC: C000 | A:00 X:00 Y:00 P:24 SP:FD"
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "PC:%04X  |  A:%02X  X:%02X  Y:%02X  P:%02X  SP:%02X",
             PC, A, X, Y, Status, SP);

    historico_log.push_back(buffer);

    // Mantém apenas as últimas 50 instruções na memória para não gastar RAM infinita
    if (historico_log.size() > 50) {
        historico_log.pop_front();
    }
}

void CPU::ImprimirHistoricoCrash() {
    std::cout << "\n=======================================================\n";
    std::cout << "    CAIXA PRETA DA CPU (Ultimas 50 instrucoes)\n";
    std::cout << "=======================================================\n";
    for (const auto& linha : historico_log) {
        std::cout << linha << "\n";
    }
    std::cout << "=======================================================\n";
}