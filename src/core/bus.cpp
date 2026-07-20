#include "bus.h"

Bus::Bus() {
    controle_estado = 0;
    estado_botoes = 0; // Inicializamos a nova variável
    controle_shift = 0;
    prg_rom_tamanho = 32768;
    chr_rom_tamanho = 8192;
    espelhamento_vertical = false;
    oam_dma_ciclos = 0;
    for (int i = 0; i < 2048; i++) cpuRam[i] = 0;
    for (int i = 0; i < 32768; i++) cartucho[i] = 0;
}

uint8_t Bus::Ler(uint16_t endereco) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        return cpuRam[endereco & 0x07FF];
    }
    else if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        return ppu.LerRegistrador(0x2000 + (endereco & 0x0007));
    }
    else if (endereco == 0x4014) {
        return 0;
    }
    else if (endereco == 0x4016) {
        // Leitura do Controle (Desloca um botão de cada vez para a CPU ler)
        uint8_t dado = (controle_estado & 0x80) > 0;
        controle_estado <<= 1;
        return dado;
    }
    else if (endereco >= 0x4000 && endereco <= 0x4017) {
        return apu.Ler(endereco);
    }
    else if (endereco >= 0x8000 && endereco <= 0xFFFF) {
        uint16_t end_mapeado = endereco & (prg_rom_tamanho > 16384 ? 0x7FFF : 0x3FFF);
        return cartucho[end_mapeado];
    }
    return 0;
}

void Bus::Escrever(uint16_t endereco, uint8_t valor) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        cpuRam[endereco & 0x07FF] = valor;
    }
    else if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        ppu.EscreverRegistrador(0x2000 + (endereco & 0x0007), valor);
    }
    else if (endereco == 0x4014) {
        oam_dma_ciclos = 513;
        for (int i = 0; i < 256; i++) {
            ppu.oam[i] = Ler((valor << 8) + i);
        }
    }
    else if (endereco == 0x4016) {
        // --- O LATCH (Trava) DO HARDWARE ---
        // O jogo sinaliza quando quer copiar os botões pressionados!
        if ((valor & 0x01) == 1) {
            controle_estado = estado_botoes;
        }
        controle_shift = valor;
    }
    else if (endereco >= 0x4000 && endereco <= 0x4017) {
        apu.Escrever(endereco, valor);
    }
}