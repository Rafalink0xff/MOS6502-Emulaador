#include "cpu.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib> // Necessário para o exit(1)

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

void CPU::ExecutarCiclo() {
    uint8_t opcode = LerMemoria(PC);
    PC++;

    switch (opcode) {

        // ==========================================
        // 1. INSTRUÇÕES DE MEMÓRIA (LOAD/STORE)
        // ==========================================
    case 0xA9: Instrucao_LDA(ModoImediato());  break; // LDA Imediato
    case 0xAD: Instrucao_LDA(ModoAbsoluto());  break; // LDA Absoluto
    case 0xBD: Instrucao_LDA(ModoAbsolutoX()); break; // LDA Absoluto,X
    case 0xA0: Instrucao_LDY(ModoImediato());  break; // LDY Imediato
    case 0x8D: Instrucao_STA(ModoAbsoluto());  break; // STA Absoluto
    case 0x85: Instrucao_STA(ModoZeroPage());  break; // STA Zero Page
    case 0x86: Instrucao_STX(ModoZeroPage());  break; // STX Zero Page

        // ==========================================
        // 2. INSTRUÇÕES DE REGISTRADORES E MATEMÁTICA
        // ==========================================
    case 0xA2: // LDX Imediato
        X = LerMemoria(PC);
        PC++;
        AtualizarFlagsZeroENegativo(X);
        break;
    case 0xAA: // TAX
        X = A;
        AtualizarFlagsZeroENegativo(X);
        break;
    case 0xE8: // INX
        X = X + 1;
        AtualizarFlagsZeroENegativo(X);
        break;
    case 0xCA: // DEX
        Instrucao_DEX();
        break;
    case 0xC9: // CMP Imediato
        Instrucao_CMP(ModoImediato());
        break;
    case 0xE0: Instrucao_CPX(ModoImediato()); break;

        // ==========================================
        // 3. DESVIOS CONDICIONAIS (BRANCHES)
        // ==========================================
    case 0x10: Instrucao_BPL(ModoRelativo()); break;
    case 0xB0: Instrucao_BCS(ModoRelativo()); break;
    case 0xD0: Instrucao_BNE(ModoRelativo()); break;

        // ==========================================
        // 4. PILHA E STATUS (FLAGS)
        // ==========================================
    case 0x9A: // TXS
        SP = X;
        break;
    case 0x78: // SEI
        Status |= 0x04;
        break;
    case 0xD8: // CLD
        Status &= ~0x08;
        break;

        // ==========================================
        // 5. CHAMADAS DE FUNÇÃO (SUB-ROTINAS)
        // ==========================================
    case 0x20: // JSR Absoluto
        Instrucao_JSR(ModoAbsoluto());
        break;

        // ==========================================
        // 6. SISTEMA
        // ==========================================
    case 0x00: // BRK
        std::cout << "Interrupcao forcada.\n";
        exit(0);

    default:
        std::cout << "ERRO FATAL: Instrucao desconhecida lida pela CPU! Opcode: 0x"
            << std::uppercase << std::hex << (int)opcode << "\n";
        exit(1);
    }
}

// ==========================================
// BARRAMENTO DE MEMÓRIA (CONTROLE DE I/O)
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
    // A pilha do 6502 fica sempre fixada na página 0x0100
    EscreverMemoria(0x0100 + SP, valor);
    SP--; // A pilha cresce para baixo!
}

// ==========================================
// MODOS DE ENDEREÇAMENTO
// ==========================================

uint16_t CPU::ModoImediato() {
    uint16_t endereco = PC;
    PC++;
    return endereco;
}

uint16_t CPU::ModoAbsoluto() {
    uint8_t byte_baixo = LerMemoria(PC);
    uint8_t byte_alto = LerMemoria(PC + 1);
    PC += 2;
    return (byte_alto << 8) | byte_baixo;
}

uint16_t CPU::ModoAbsolutoX() {
    uint8_t byte_baixo = LerMemoria(PC);
    uint8_t byte_alto = LerMemoria(PC + 1);
    PC += 2;
    uint16_t endereco_base = (byte_alto << 8) | byte_baixo;
    return endereco_base + X;
}

uint16_t CPU::ModoRelativo() {
    int8_t deslocamento = (int8_t)LerMemoria(PC);
    PC++;
    return PC + deslocamento;
}

uint16_t CPU::ModoZeroPage() {
    // Lê apenas 1 byte do endereço
    uint16_t endereco = LerMemoria(PC);
    PC++;

    // O retorno já é o endereço final, pois C++ preenche a parte alta com zeros naturalmente!
    return endereco;
}

// ==========================================
// LÓGICA DAS INSTRUÇÕES
// ==========================================

void CPU::AtualizarFlagsZeroENegativo(uint8_t registrador) {
    if (registrador == 0x00) { Status |= 0x02; }
    else { Status &= ~0x02; }

    if (registrador & 0x80) { Status |= 0x80; }
    else { Status &= ~0x80; }
}

void CPU::Instrucao_LDA(uint16_t endereco) {
    A = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_LDY(uint16_t endereco) {
    Y = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(Y);
}

void CPU::Instrucao_STA(uint16_t endereco) {
    EscreverMemoria(endereco, A);
}

void CPU::Instrucao_STX(uint16_t endereco) {
    EscreverMemoria(endereco, X);
}

void CPU::Instrucao_BPL(uint16_t endereco) {
    if ((Status & 0x80) == 0) { PC = endereco; }
}

void CPU::Instrucao_BCS(uint16_t endereco) {
    if ((Status & 0x01) == 1) { PC = endereco; }
}

void CPU::Instrucao_CMP(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = A - valor;

    if (A >= valor) { Status |= 0x01; }
    else { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_CPX(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = X - valor;

    if (X >= valor) { Status |= 0x01; } // Acende o Carry
    else { Status &= ~0x01; } // Apaga o Carry

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_DEX() {
    X = X - 1;
    AtualizarFlagsZeroENegativo(X);
}

void CPU::Instrucao_BNE(uint16_t endereco) {
    // BNE desvia se a luz Zero (Bit 1) estiver APAGADA (igual a 0)
    if ((Status & 0x02) == 0) {
        PC = endereco;
    }
}

void CPU::Instrucao_JSR(uint16_t endereco) {
    // 1. O PC neste momento já avançou para a próxima instrução.
    // A regra física maluca do 6502 exige que salvemos o endereço de retorno MENOS 1.
    uint16_t endereco_retorno = PC - 1;

    // 2. Quebramos o endereço de 16 bits ao meio e salvamos na Pilha.
    // O 6502 salva primeiro a metade alta (High Byte), depois a baixa (Low Byte).
    Push((endereco_retorno >> 8) & 0xFF);
    Push(endereco_retorno & 0xFF);

    // 3. Agora que sabemos como voltar, pulamos para a função!
    PC = endereco;
}

// ==========================================
// DISASSEMBLER (FERRAMENTA DE DIAGNÓSTICO)
// ==========================================

std::string CPU::DisassembleInstrucao(uint16_t& endereco) {
    uint8_t opcode = memoria[endereco];
    std::stringstream ss;

    ss << "[" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << endereco << "] ";

    switch (opcode) {
        // --- 1 Byte ---
    case 0xAA: ss << "AA       TAX"; endereco += 1; break;
    case 0x9A: ss << "9A       TXS"; endereco += 1; break;
    case 0xE8: ss << "E8       INX"; endereco += 1; break;
    case 0xCA: ss << "CA       DEX"; endereco += 1; break;
    case 0x78: ss << "78       SEI"; endereco += 1; break;
    case 0xD8: ss << "D8       CLD"; endereco += 1; break;

        // --- 2 Bytes ---
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

    // --- 3 Bytes ---
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
    case 0x20: // JSR Absoluto
    {
        uint8_t l = memoria[endereco + 1]; uint8_t h = memoria[endereco + 2];
        ss << "20 " << std::setfill('0') << std::setw(2) << (int)l << " " << std::setw(2) << (int)h
            << "    JSR $" << std::setw(2) << (int)h << std::setw(2) << (int)l;
        endereco += 3; break;
    }

    default:
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)opcode << "       ??? (Nao Implementado)";
        endereco += 1;
        break;
    }
    return ss.str();
}