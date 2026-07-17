#include "bus.h"

Bus::Bus() {
    for (int i = 0; i < 2048; i++) cpuRam[i] = 0;
    for (int i = 0; i < 32768; i++) cartucho[i] = 0;
}

uint8_t Bus::Ler(uint16_t endereco) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        return cpuRam[endereco & 0x07FF];
    }
    if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        return ppu.LerRegistrador(endereco & 0x2007); // Rota para a PPU!
    }
    if (endereco >= 0x8000 && endereco <= 0xFFFF) {
        return cartucho[endereco - 0x8000];
    }
    return 0;
}

void Bus::Escrever(uint16_t endereco, uint8_t valor) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        cpuRam[endereco & 0x07FF] = valor;
    }
    else if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        ppu.EscreverRegistrador(endereco & 0x2007, valor); // Rota para a PPU!
    }
}