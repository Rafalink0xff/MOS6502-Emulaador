#include "cpu.h"

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
    return ((byte_alto << 8) | byte_baixo) + X;
}

uint16_t CPU::ModoAbsolutoY() {
    uint8_t byte_baixo = LerMemoria(PC);
    uint8_t byte_alto = LerMemoria(PC + 1);
    PC += 2;
    uint16_t endereco_base = (byte_alto << 8) | byte_baixo;
    return endereco_base + Y;
}

uint16_t CPU::ModoRelativo() {
    int8_t deslocamento = (int8_t)LerMemoria(PC);
    PC++;
    return PC + deslocamento;
}

uint16_t CPU::ModoZeroPage() {
    uint16_t endereco = LerMemoria(PC);
    PC++;
    return endereco;
}

uint16_t CPU::ModoIndiretoY() {
    uint8_t ponteiro_zp = LerMemoria(PC);
    PC++;

    uint8_t byte_baixo = LerMemoria(ponteiro_zp);
    uint8_t byte_alto = LerMemoria((uint8_t)(ponteiro_zp + 1));

    uint16_t endereco_base = (byte_alto << 8) | byte_baixo;
    return endereco_base + Y;
}