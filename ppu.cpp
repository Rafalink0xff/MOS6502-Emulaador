#include "ppu.h"
#include <SDL2/SDL.h>

struct RGB { uint8_t r, g, b; };
static const RGB NES_PALETTE[64] = {
    {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0},
    {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0},
    {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
    {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},
    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
    {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
};

PPU::PPU() {
    ppu_scroll_x = 0;
    ppu_scroll_y = 0;
    nmi_ocorreu = false;
    ppu_ctrl = 0;
    vram_addr = 0;
    latch = false;
    scanline = 0;
    ciclo = 0;
    sprite0_hit = false;
    for (int i = 0; i < 2048; i++) vram[i] = 0;
    for (int i = 0; i < 32; i++) paleta[i] = 0;
    for (int i = 0; i < 256; i++) oam[i] = 0;
}

bool PPU::Clock() {
    bool frame_pronto = false;

    if (scanline >= 0 && scanline < 240 && ciclo >= 1 && ciclo <= 256) {
        if (!sprite0_hit) {
            int sprite_y = oam[0];
            int sprite_tile = oam[1];
            int sprite_atributo = oam[2];
            int sprite_x = oam[3];

            int pixel_x = ciclo - 1;
            int pixel_y = scanline;

            if (pixel_x >= sprite_x && pixel_x < sprite_x + 8 &&
                pixel_y >= sprite_y && pixel_y < sprite_y + 8) {
                uint16_t padrao_banco = (ppu_ctrl & 0x08) ? 4096 : 0;
                uint16_t tile_endereco = padrao_banco + (sprite_tile * 16);
                int linha_real = (sprite_atributo & 0x80) ? (7 - (pixel_y - sprite_y)) : (pixel_y - sprite_y);
                int bit_real = (sprite_atributo & 0x40) ? (7 - (pixel_x - sprite_x)) : (pixel_x - sprite_x);
                uint8_t byte1 = chr_rom[tile_endereco + linha_real];
                uint8_t byte2 = chr_rom[tile_endereco + linha_real + 8];
                uint8_t color_val = (((byte2 >> (7 - bit_real)) & 1) << 1) | ((byte1 >> (7 - bit_real)) & 1);
                if (color_val != 0) {
                    sprite0_hit = true;
                }
            }
        }
    }

    ciclo++;
    if (ciclo > 340) {
        ciclo = 0;
        scanline++;

        if (scanline == 241) {
            nmi_ocorreu = true;
        }

        if (scanline > 261) {
            scanline = 0;
            sprite0_hit = false;
            nmi_ocorreu = false;
            frame_pronto = true;
        }
    }

    return frame_pronto;
}

uint8_t PPU::CorBackgroundEmPixel(int pixel_x, int pixel_y) const {
    int scroll_x_total = ppu_scroll_x + ((ppu_ctrl & 0x03) & 1) * 256;
    int scroll_y_total = ppu_scroll_y;

    int x_mundo = (pixel_x + scroll_x_total) & 0x1FF;
    int y_mundo = pixel_y + scroll_y_total;

    int nametable_offset = (x_mundo >= 256) ? 1024 : 0;
    int tile_x = (x_mundo % 256) / 8;
    int tile_y = (y_mundo / 8) % 30;
    int linha = y_mundo % 8;

    int vram_nametable_base = nametable_offset;
    int vram_index = vram_nametable_base + (tile_y * 32) + tile_x;
    uint8_t tile_id = vram[vram_index & 0x07FF];
    uint16_t padrao_banco = (ppu_ctrl & 0x10) ? 4096 : 0;
    uint16_t tile_endereco = padrao_banco + (tile_id * 16);

    int atributo_index = vram_nametable_base + 960 + ((tile_y / 4) * 8) + (tile_x / 4);
    uint8_t atributo_byte = vram[atributo_index & 0x07FF];
    int shift = (((tile_y % 4) / 2) * 4) + (((tile_x % 4) / 2) * 2);
    uint8_t paleta_id = (atributo_byte >> shift) & 0x03;

    uint8_t byte1 = chr_rom[tile_endereco + linha];
    uint8_t byte2 = chr_rom[tile_endereco + linha + 8];
    int bit = 7 - (x_mundo % 8);
    uint8_t color_bit1 = (byte1 >> bit) & 1;
    uint8_t color_bit2 = (byte2 >> bit) & 1;
    uint8_t color_val = (color_bit2 << 1) | color_bit1;
    uint16_t paleta_endereco = (paleta_id * 4) + color_val;
    if (color_val == 0) paleta_endereco = 0;

    return paleta[paleta_endereco & 0x1F];
}

uint8_t PPU::CorSprite0EmPixel(int pixel_x, int pixel_y) const {
    uint8_t sprite_y = oam[0];
    uint8_t tile_id = oam[1];
    uint8_t atributo = oam[2];
    uint8_t sprite_x = oam[3];

    if (pixel_x < sprite_x || pixel_x >= sprite_x + 8) return 0xFF;
    if (pixel_y < sprite_y || pixel_y >= sprite_y + 8) return 0xFF;

    uint16_t padrao_banco = (ppu_ctrl & 0x08) ? 4096 : 0;
    uint16_t tile_endereco = padrao_banco + (tile_id * 16);
    int linha = pixel_y - sprite_y;
    int coluna = pixel_x - sprite_x;

    if (atributo & 0x80) linha = 7 - linha;
    if (atributo & 0x40) coluna = 7 - coluna;

    uint8_t byte1 = chr_rom[tile_endereco + linha];
    uint8_t byte2 = chr_rom[tile_endereco + linha + 8];
    uint8_t color_val = (((byte2 >> (7 - coluna)) & 1) << 1) | ((byte1 >> (7 - coluna)) & 1);
    if (color_val == 0) return 0xFF;

    uint8_t paleta_id = (atributo & 0x03) + 4;
    uint16_t paleta_endereco = (paleta_id * 4) + color_val;
    return paleta[paleta_endereco & 0x1F];
}

void PPU::Renderizar(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < 240; y++) {
        for (int x = 0; x < 256; x++) {
            uint8_t color = CorBackgroundEmPixel(x, y);
            uint8_t sprite_color = CorSprite0EmPixel(x, y);
            if (sprite_color != 0xFF) color = sprite_color;

            RGB rgb = NES_PALETTE[color & 0x3F];
            SDL_SetRenderDrawColor(renderer, rgb.r, rgb.g, rgb.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    SDL_RenderPresent(renderer);
}

uint8_t PPU::LerRegistrador(uint16_t endereco) {
    uint8_t dado = 0x00;

    switch (endereco) {
        case 0x2002: // PPUSTATUS
            if (nmi_ocorreu) dado |= 0x80;
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
            if (vram_addr >= 0x3F00 && vram_addr <= 0x3FFF) {
                paleta[vram_addr & 0x001F] = valor;
            } else {
                vram[vram_addr & 0x07FF] = valor;
            }

            if (ppu_ctrl & 0x04) vram_addr += 32;
            else                 vram_addr += 1;
            break;
    }
}