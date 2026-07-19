#pragma once
#include <cstdint>
#include "ppu.h"
#include "apu.h" // <-- Incluímos a APU

class Bus {
public:
    Bus();
    uint8_t cpuRam[2048];
    uint8_t cartucho[32768];

    uint16_t prg_rom_tamanho;
    uint16_t chr_rom_tamanho;
    bool espelhamento_vertical;
    uint16_t oam_dma_ciclos;

    PPU ppu;
    APU apu; // <-- Instanciamos a placa de som

    uint8_t controle_estado;
    uint8_t controle_shift;

    void Escrever(uint16_t endereco, uint8_t valor);
    uint8_t Ler(uint16_t endereco);
};