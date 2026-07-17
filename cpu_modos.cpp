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

uint16_t CPU::ModoZeroPageX() {
    // Lê o endereço base, soma o X e usa a máscara & 0xFF para garantir
    // que, se passar de 255, ele "dê a volta" dentro da Zero Page.
    uint16_t endereco = (LerMemoria(PC) + X) & 0x00FF;
    PC++;
    return endereco;
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

uint16_t CPU::ModoIndiretoX() {
    // 1. Lê o valor base na Zero Page
    uint8_t ponteiro_zp = LerMemoria(PC);
    PC++;

    // 2. Soma o X ao ponteiro ANTES de ler o endereço (wrap-around na Zero Page)
    // Usamos uint8_t para garantir que ele dê a volta em 255 e não saia da Zero Page
    uint8_t ponteiro_real = ponteiro_zp + X;

    // 3. Lê o endereço de 16 bits que está guardado lá
    uint8_t byte_baixo = LerMemoria(ponteiro_real);
    uint8_t byte_alto = LerMemoria((uint8_t)(ponteiro_real + 1));

    // 4. Retorna o endereço final
    return (byte_alto << 8) | byte_baixo;
}