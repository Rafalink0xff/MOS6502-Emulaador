#include "ppu.h"

PPU::PPU() {
    ppu_scroll_x = 0;
    ppu_scroll_y = 0;
    nmi_ocorreu = false;
    ppu_ctrl = 0;
    vram_addr = 0; // ADICIONE ISSO
    latch = false; // ADICIONE ISSO
    for (int i = 0; i < 2048; i++) vram[i] = 0;
    for (int i = 0; i < 32; i++) paleta[i] = 0;
    for (int i = 0; i < 256; i++) oam[i] = 0;
}

uint8_t PPU::LerRegistrador(uint16_t endereco) {
    // ... (Seu código do LerRegistrador continua EXATAMENTE igual) ...
    uint8_t dado = 0x00;

    switch (endereco) {
        case 0x2002: // PPUSTATUS
            if (nmi_ocorreu) dado |= 0x80;

            static bool sprite0_hit = false;
            sprite0_hit = !sprite0_hit;
            if (sprite0_hit) dado |= 0x40;

            nmi_ocorreu = false;
            latch = false; // <-- ADICIONE ISSO! LER O $2002 DESTRAVA O LATCH!
            break;
    }
    return dado;
}

void PPU::EscreverRegistrador(uint16_t endereco, uint8_t valor) {
    switch (endereco) {
        case 0x2000: // PPUCTRL
            ppu_ctrl = valor;
            break;
        case 0x2005: // PPUSCROLL
            if (latch == false) {
                ppu_scroll_x = valor; // Primeira escrita = Posição X
                latch = true;
            } else {
                ppu_scroll_y = valor; // Segunda escrita = Posição Y
                latch = false;
            }
            break;
        case 0x2006: // PPUADDR
            if (latch == false) {
                // Primeira escrita: Pega o byte ALTO. Limpa os bits altos antigos e injeta os novos.
                vram_addr = (uint16_t)((valor & 0x3F) << 8) | (vram_addr & 0x00FF);
                latch = true; // Muda a trava para a próxima escrita
            } else {
                // Segunda escrita: Pega o byte BAIXO. Limpa os bits baixos antigos e injeta os novos.
                vram_addr = (vram_addr & 0xFF00) | valor;
                latch = false; // Destrava
            }
            break;
        case 0x2007: // PPUDATA
            // Se o endereço estiver na área das Paletas ($3F00 a $3FFF)
            if (vram_addr >= 0x3F00 && vram_addr <= 0x3FFF) {
                // A memória de paleta tem só 32 bytes, então usamos a máscara 0x001F
                paleta[vram_addr & 0x001F] = valor;
            } else {
                // Caso contrário, é VRAM normal (Nametable)
                vram[vram_addr & 0x07FF] = valor;
            }

            // A PPU avança o endereço sozinha após cada escrita!
            if (ppu_ctrl & 0x04) vram_addr += 32;
            else                 vram_addr += 1;
            break;
    }
}