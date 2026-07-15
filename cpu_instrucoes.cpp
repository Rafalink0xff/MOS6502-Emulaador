#include "cpu.h"

void CPU::AtualizarFlagsZeroENegativo(uint8_t registrador) {
    if (registrador == 0x00) { Status |= 0x02; }
    else                     { Status &= ~0x02; }

    if (registrador & 0x80)  { Status |= 0x80; }
    else                     { Status &= ~0x80; }
}

void CPU::Instrucao_LDA(uint16_t endereco) {
    A = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_LDX(uint16_t endereco) {
    X = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(X);
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

void CPU::Instrucao_JMP(uint16_t endereco) {
    PC = endereco;
}

void CPU::Instrucao_BPL(uint16_t endereco) {
    if ((Status & 0x80) == 0) { PC = endereco; }
}

void CPU::Instrucao_BCS(uint16_t endereco) {
    if ((Status & 0x01) == 1) { PC = endereco; }
}

void CPU::Instrucao_BNE(uint16_t endereco) {
    if ((Status & 0x02) == 0) { PC = endereco; }
}

void CPU::Instrucao_CMP(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = A - valor;

    if (A >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_CPX(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = X - valor;

    if (X >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_CPY(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = Y - valor;

    if (Y >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_BIT(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);

    uint8_t resultado_and = A & valor;
    if (resultado_and == 0) { Status |= 0x02;  }
    else                    { Status &= ~0x02; }

    if (valor & 0x80) { Status |= 0x80;  }
    else              { Status &= ~0x80; }

    if (valor & 0x40) { Status |= 0x40;  }
    else              { Status &= ~0x40; }
}

void CPU::Instrucao_AND(uint16_t endereco) {
    A = A & LerMemoria(endereco);

    if (A == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (A & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_ORA(uint16_t endereco) {
    A = A | LerMemoria(endereco);

    if (A == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (A & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_INC(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    valor++;
    EscreverMemoria(endereco, valor);

    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_DEX() {
    X = X - 1;
    AtualizarFlagsZeroENegativo(X);
}

void CPU::Instrucao_DEY() {
    Y = Y - 1;
    AtualizarFlagsZeroENegativo(Y);
}

void CPU::Instrucao_INY(uint16_t endereco) {
    Y++;
    if (Y == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (Y & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_JSR(uint16_t endereco) {
    uint16_t endereco_retorno = PC - 1;
    Push((endereco_retorno >> 8) & 0xFF);
    Push(endereco_retorno & 0xFF);
    PC = endereco;
}

void CPU::Instrucao_RTS() {
    uint8_t byte_baixo = Pop();
    uint8_t byte_alto = Pop();
    uint16_t endereco_retorno = (byte_alto << 8) | byte_baixo;
    PC = endereco_retorno + 1;
}