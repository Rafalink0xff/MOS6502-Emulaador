#include "cpu.h"
#include <iostream>
#include <sstream>
#include <iomanip>

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
    uint8_t opcode = LerMemoria(PC); // A CPU lê o comando passando pelo pedágio!
    PC++;

    switch (opcode) {

        // --- INSTRUÇÕES DE CARREGAMENTO (LDA) ---
    case 0xA9: // LDA Modo Imediato
        Instrucao_LDA(ModoImediato());
        break;

    case 0xA0: // LDY Imediato
        Instrucao_LDY(ModoImediato());
        break;

    case 0xBD: // LDA Modo Absoluto,X
        Instrucao_LDA(ModoAbsolutoX());
        break;

    case 0xAD: // LDA Modo Absoluto
        Instrucao_LDA(ModoAbsoluto());
        break;

        // --- INSTRUÇÕES DE GRAVAÇÃO (STA) ---
    case 0x8D: // STA Modo Absoluto
        Instrucao_STA(ModoAbsoluto());
        break;

        // --- INSTRUÇÕES COM REGISTRADOR X ---
    case 0xA2: // LDX Imediato
    {
        uint8_t valor = LerMemoria(PC);
        X = valor;
        PC++;
        AtualizarFlagsZeroENegativo(X);
        break;
    }
    case 0xAA: // TAX (Transfer A to X)
    {
        X = A;
        AtualizarFlagsZeroENegativo(X);
        break;
    }
    case 0xE8: // INX (Increment X)
    {
        X = X + 1;
        AtualizarFlagsZeroENegativo(X);
        break;
    }

    // --- INSTRUÇÕES DE DESVIO (BRANCHES) ---
    case 0x10: // BPL Modo Relativo
        Instrucao_BPL(ModoRelativo());
        break;

        // --- INSTRUÇÕES DE PILHA (STACK) ---
    case 0x9A: // TXS (Transfer X to Stack Pointer)
    {
        SP = X;
        break;
    }

    // --- INSTRUÇÕES DE STATUS (FLAGS) ---
    case 0x78: // SEI (Set Interrupt Disable)
    {
        Status |= 0x04;
        break;
    }
    case 0xD8: // CLD (Clear Decimal Mode)
    {
        Status &= ~0x08;
        break;
    }

    // --- CONTROLE DE FLUXO ---
    case 0x00: // BRK (Force Interrupt / Fim)
    {
        std::cout << "Interrupcao forcada.\n";
        break;
    }

    default:
        std::cout << "ERRO FATAL: Instrucao desconhecida lida pela CPU!\n";
        break;
    }
}

// ==========================================
// BARRAMENTO DE MEMÓRIA (CONTROLE DE I/O)
// ==========================================

uint8_t CPU::LerMemoria(uint16_t endereco) {
    if (endereco == 0x2002) {
        return 0x80; // HACK DO VBLANK: Mente para o Mario dizendo que a tela já está pronta
    }
    return memoria[endereco];
}

void CPU::EscreverMemoria(uint16_t endereco, uint8_t valor) {
    memoria[endereco] = valor;
}

// ==========================================
// MODOS DE ENDEREÇAMENTO E INSTRUÇÕES
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

    // O grande truque: O endereço real é o endereço base somado ao valor de X!
    return endereco_base + X;
}

uint16_t CPU::ModoRelativo() {
    int8_t deslocamento = (int8_t)LerMemoria(PC);
    PC++;
    return PC + deslocamento;
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

void CPU::Instrucao_BPL(uint16_t endereco) {
    // Se a Flag Negativa (bit 7) estiver 0, o salto acontece!
    if ((Status & 0x80) == 0) {
        PC = endereco;
    }
}

void CPU::AtualizarFlagsZeroENegativo(uint8_t registrador) {
    if (registrador == 0x00) {
        Status |= 0x02;
    }
    else {
        Status &= ~0x02;
    }

    if (registrador & 0x80) {
        Status |= 0x80;
    }
    else {
        Status &= ~0x80;
    }
}

// ==========================================
// DISASSEMBLER (FERRAMENTA DE DIAGNÓSTICO)
// ==========================================

std::string CPU::DisassembleInstrucao(uint16_t& endereco) {
    uint8_t opcode = memoria[endereco];
    std::stringstream ss;

    ss << "[" << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << endereco << "] ";

    switch (opcode) {
    case 0xA9: // LDA Immediate
    {
        uint8_t valor = memoria[endereco + 1];
        ss << "A9 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDA #$" << (int)valor;
        endereco += 2;
        break;
    }
    case 0xA0: // LDY Immediate
    {
        uint8_t valor = memoria[endereco + 1];
        ss << "A0 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDY #$" << (int)valor;
        endereco += 2; // Aqui nós dizemos ao leitor para pular 2 bytes, evitando o desalinhamento!
        break;
    }
    case 0xAD: // LDA Absoluto
    {
        uint8_t byte_baixo = memoria[endereco + 1];
        uint8_t byte_alto = memoria[endereco + 2];
        ss << "AD " << std::setfill('0') << std::setw(2) << (int)byte_baixo << " " << std::setfill('0') << std::setw(2) << (int)byte_alto
            << "    LDA $" << std::setfill('0') << std::setw(2) << (int)byte_alto << std::setfill('0') << std::setw(2) << (int)byte_baixo;
        endereco += 3;
        break;
    }
    case 0xBD: // LDA Absoluto,X
    {
        uint8_t byte_baixo = memoria[endereco + 1];
        uint8_t byte_alto = memoria[endereco + 2];
        ss << "BD " << std::setfill('0') << std::setw(2) << (int)byte_baixo << " " << std::setfill('0') << std::setw(2) << (int)byte_alto
            << "    LDA $" << std::setfill('0') << std::setw(2) << (int)byte_alto << std::setfill('0') << std::setw(2) << (int)byte_baixo << ",X";
        endereco += 3;
        break;
    }
    case 0x8D: // STA Absoluto
    {
        uint8_t byte_baixo = memoria[endereco + 1];
        uint8_t byte_alto = memoria[endereco + 2];
        ss << "8D " << std::setfill('0') << std::setw(2) << (int)byte_baixo << " " << std::setfill('0') << std::setw(2) << (int)byte_alto
            << "    STA $" << std::setfill('0') << std::setw(2) << (int)byte_alto << std::setfill('0') << std::setw(2) << (int)byte_baixo;
        endereco += 3;
        break;
    }
    case 0xA2: // LDX Immediate
    {
        uint8_t valor = memoria[endereco + 1];
        ss << "A2 " << std::setfill('0') << std::setw(2) << (int)valor << "    LDX #$" << (int)valor;
        endereco += 2;
        break;
    }
    case 0x10: // BPL Relativo
    {
        int8_t deslocamento = (int8_t)memoria[endereco + 1];
        uint16_t destino = endereco + 2 + deslocamento;
        ss << "10 " << std::setfill('0') << std::setw(2) << (int)(uint8_t)deslocamento
            << "    BPL $" << std::setfill('0') << std::setw(4) << destino;
        endereco += 2;
        break;
    }
    case 0xAA: ss << "AA       TAX"; endereco += 1; break;
    case 0x9A: ss << "9A       TXS"; endereco += 1; break;
    case 0xE8: ss << "E8       INX"; endereco += 1; break;
    case 0x78: ss << "78       SEI"; endereco += 1; break;
    case 0xD8: ss << "D8       CLD"; endereco += 1; break;
    default:
        ss << std::setfill('0') << std::setw(2) << std::hex << (int)opcode << "       ??? (Nao Implementado)";
        endereco += 1;
        break;
    }
    return ss.str();
}