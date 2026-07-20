#pragma once
#include <cstdint>
#include "ppu.h"
#include "apu.h"

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
    APU apu;

    uint8_t controle_estado; // O que a CPU enxerga (desloca 1 bit por vez)
    uint8_t estado_botoes;   // O Buffer seguro do teclado (sempre atualizado pelo frontend)
    uint8_t controle_shift;

    void Escrever(uint16_t endereco, uint8_t valor);
    uint8_t Ler(uint16_t endereco);
};