#pragma once
#include <cstdint>
#include "ppu.h"

class Bus {
public:
    Bus();
    uint8_t cpuRam[2048];    // 2KB de RAM física do NES
    uint8_t cartucho[32768]; // 32KB de ROM do Cartucho

    PPU ppu;

    void Escrever(uint16_t endereco, uint8_t valor);
    uint8_t Ler(uint16_t endereco);
};