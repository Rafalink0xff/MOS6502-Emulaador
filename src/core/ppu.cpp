#include "ppu.h"
#include <SDL2/SDL.h>
#include <iostream>

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
    render_scroll_x = 0;
    render_scroll_y = 0;
    render_ctrl = 0;

    ppu_scroll_x = 0;
    ppu_scroll_y = 0;
    oam_addr = 0;
    nmi_ocorreu = false;
    vblank_ativo = false;
    ppu_ctrl = 0;
    vram_addr = 0;
    latch = false;
    scanline = 0;
    ciclo = 0;
    sprite0_hit = false;
    bg_transparente = false;
    espelhamento_vertical = false;
    leitura_buffer = 0;

    for (int i = 0; i < 2048; i++) vram[i] = 0;
    for (int i = 0; i < 32; i++) paleta[i] = 0x0F;
    for (int i = 0; i < 256; i++) oam[i] = 0;
    for (int i = 0; i < (256 * 240); i++) tela_pixels[i] = 0x0F;
}

void PPU::DefinirEspelhamentoVertical(bool vertical) {
    espelhamento_vertical = vertical;
}

uint8_t PPU::CorPaleta(uint8_t indice) const {
    return paleta[MapearPaleta(indice & 0x1F)] & 0x3F;
}

uint16_t PPU::MapearPaleta(uint16_t endereco) const {
    uint16_t indice = endereco & 0x1F;
    switch (indice) {
        case 0x10: return 0x00;
        case 0x14: return 0x04;
        case 0x18: return 0x08;
        case 0x1C: return 0x0C;
        default: return indice;
    }
}

uint16_t PPU::MapearNametable(uint16_t endereco) const {
    uint16_t indice = (endereco - 0x2000) & 0x0FFF;

    if (espelhamento_vertical) {
        // Vertical Mirroring: Telas da esquerda e direita são independentes
        return indice % 0x800;
    } else {
        // Horizontal Mirroring: Telas de cima e baixo são independentes
        if (indice < 0x800) return indice & 0x3FF;
        else return 0x400 + (indice & 0x3FF);
    }
}

uint8_t PPU::LerChrRomSeguro(uint16_t endereco) const {
    if (endereco >= 8192) return 0;
    return chr_rom[endereco];
}

uint8_t PPU::LerVramSeguro(uint16_t endereco) const {
    return vram[endereco & 0x07FF];
}

uint8_t PPU::LerMemoriaPPU(uint16_t endereco) const {
    if (endereco >= 0x3F00 && endereco <= 0x3FFF) {
        return paleta[MapearPaleta(endereco)];
    }
    if (endereco >= 0x3000 && endereco <= 0x3EFF) {
        endereco -= 0x1000;
    }
    if (endereco >= 0x2000 && endereco <= 0x2FFF) {
        return LerVramSeguro(MapearNametable(endereco));
    }
    if (endereco <= 0x1FFF) {
        return LerChrRomSeguro(endereco);
    }
    return 0;
}

void PPU::EscreverMemoriaPPU(uint16_t endereco, uint8_t valor) {
    if (endereco >= 0x3F00 && endereco <= 0x3FFF) {
        paleta[MapearPaleta(endereco)] = valor;
        return;
    }
    if (endereco >= 0x3000 && endereco <= 0x3EFF) {
        endereco -= 0x1000;
    }
    if (endereco >= 0x2000 && endereco <= 0x2FFF) {
        vram[MapearNametable(endereco)] = valor;
    }
}

uint8_t PPU::CorBackgroundEmPixel(int pixel_x, int pixel_y) const {
    int nt_x = (render_ctrl & 0x01);
    int nt_y = (render_ctrl & 0x02) >> 1;

    int total_scroll_x = pixel_x + render_scroll_x;
    int total_scroll_y = pixel_y + render_scroll_y;

    nt_x = (nt_x + (total_scroll_x / 256)) % 2;
    nt_y = (nt_y + (total_scroll_y / 240)) % 2;

    int nametable_id = nt_x | (nt_y << 1);

    int tile_x = (total_scroll_x % 256) / 8;
    int tile_y = (total_scroll_y % 240) / 8;
    int linha = total_scroll_y % 8;

    uint16_t endereco_nametable = 0x2000 + (nametable_id * 0x400) + (tile_y * 32) + tile_x;
    uint8_t tile_id = LerVramSeguro(MapearNametable(endereco_nametable));

    uint16_t padrao_banco = (render_ctrl & 0x10) ? 4096 : 0;
    uint16_t tile_endereco = padrao_banco + (tile_id * 16);

    uint8_t byte1 = LerChrRomSeguro(tile_endereco + linha);
    uint8_t byte2 = LerChrRomSeguro(tile_endereco + linha + 8);
    int bit = 7 - (total_scroll_x % 8);
    uint8_t color_bit1 = (byte1 >> bit) & 1;
    uint8_t color_bit2 = (byte2 >> bit) & 1;
    uint8_t c = (color_bit2 << 1) | color_bit1;

    // --- LÓGICA DE TRANSPARÊNCIA ---
    if (c == 0) {
        bg_transparente = true;
        return LerMemoriaPPU(0x3F00); // Fundo azul/transparente
    }

    bg_transparente = false;

    uint16_t endereco_atributo = 0x2000 + (nametable_id * 0x400) + 960 + ((tile_y / 4) * 8) + (tile_x / 4);
    uint8_t atributo_byte = LerVramSeguro(MapearNametable(endereco_atributo));
    int shift = (((tile_y % 4) / 2) * 4) + (((tile_x % 4) / 2) * 2);
    uint8_t paleta_id = (atributo_byte >> shift) & 0x03;

    return LerMemoriaPPU(0x3F01 + (paleta_id * 4) + (c - 1));
}

uint8_t PPU::CorSpriteEmPixel(int pixel_x, int pixel_y, int indice_sprite) const {
    int base = indice_sprite * 4;
    uint8_t sprite_y = oam[base + 0];
    uint8_t tile_id = oam[base + 1];
    uint8_t atributo = oam[base + 2];
    uint8_t sprite_x = oam[base + 3];

    // O Sprite é renderizado a partir da posição Y+1 no NES
    if (pixel_x < sprite_x || pixel_x >= sprite_x + 8) return 0xFF;
    if (pixel_y <= sprite_y || pixel_y > sprite_y + 8) return 0xFF;

    uint16_t padrao_banco = (render_ctrl & 0x08) ? 4096 : 0;
    uint16_t tile_endereco = padrao_banco + (tile_id * 16);

    int linha = (pixel_y - 1) - sprite_y;
    int coluna = pixel_x - sprite_x;

    if (atributo & 0x80) linha = 7 - linha; // Flip Vertical
    if (atributo & 0x40) coluna = 7 - coluna; // Flip Horizontal

    uint8_t byte1 = LerChrRomSeguro(tile_endereco + linha);
    uint8_t byte2 = LerChrRomSeguro(tile_endereco + linha + 8);
    uint8_t color_val = (((byte2 >> (7 - coluna)) & 1) << 1) | ((byte1 >> (7 - coluna)) & 1);

    if (color_val == 0) return 0xFF;

    uint8_t paleta_id = (atributo & 0x03) + 4;
    uint16_t paleta_endereco = (paleta_id * 4) + color_val;
    return CorPaleta(static_cast<uint8_t>(paleta_endereco));
}

bool PPU::Clock() {
    bool frame_pronto = false;

    if (scanline >= 0 && scanline < 240 && ciclo >= 1 && ciclo <= 256) {
        int pixel_x = ciclo - 1;
        int pixel_y = scanline;

        // Trava o Scroll no começo de cada linha para evitar tela tremendo (Tearing)
        if (pixel_x == 0) {
            render_scroll_x = ppu_scroll_x;
            render_scroll_y = ppu_scroll_y;
            render_ctrl = ppu_ctrl;
        }

        uint8_t cor_bg = CorBackgroundEmPixel(pixel_x, pixel_y);
        uint8_t cor_sprite = 0xFF;

        for (int i = 63; i >= 0; i--) {
            int base = i * 4;
            uint8_t sy = oam[base + 0];
            if (sy >= 239) continue;
            uint8_t sx = oam[base + 3];

            if (pixel_x >= sx && pixel_x < sx + 8 && pixel_y > sy && pixel_y <= sy + 8) {
                uint8_t color = CorSpriteEmPixel(pixel_x, pixel_y, i);
                if (color != 0xFF) {
                    cor_sprite = color;

                    // A DETECÇÃO PERFEITA DO SPRITE 0
                    if (i == 0 && !bg_transparente && pixel_x < 255) {
                        sprite0_hit = true;
                    }
                }
            }
        }

        if (cor_sprite != 0xFF) {
            tela_pixels[pixel_y * 256 + pixel_x] = cor_sprite;
        } else {
            tela_pixels[pixel_y * 256 + pixel_x] = cor_bg;
        }

        // --- GATILHO DE EMERGÊNCIA (Failsafe) ---
        // Se a CPU travar esperando o Sprite 0, nós liberamos ela no fim da tela!
        if (pixel_x == 255 && pixel_y == 239 && !sprite0_hit) {
            sprite0_hit = true;
        }
    }

    ciclo++;
    if (ciclo > 340) {
        ciclo = 0;
        scanline++;

        if (scanline == 241) {
            nmi_ocorreu = true;
            vblank_ativo = true;
        }

        if (scanline > 261) {
            scanline = 0;
            sprite0_hit = false;
            nmi_ocorreu = false;
            vblank_ativo = false;
            frame_pronto = true;
        }
    }

    return frame_pronto;
}

void PPU::Renderizar(SDL_Texture* textura) const {
    static uint32_t pixels[256 * 240];

    for (int i = 0; i < 256 * 240; i++) {
        uint8_t color = tela_pixels[i];
        RGB rgb = NES_PALETTE[color & 0x3F];
        pixels[i] = 0xFF000000 | (rgb.r << 16) | (rgb.g << 8) | rgb.b;
    }
    // Ela agora só atualiza a textura, não joga mais na tela sozinha.
    SDL_UpdateTexture(textura, NULL, pixels, 256 * sizeof(uint32_t));
}

uint8_t PPU::LerRegistrador(uint16_t endereco) {
    uint8_t dado = 0x00;

    switch (endereco) {
        case 0x2002: // PPUSTATUS
            if (vblank_ativo) dado |= 0x80;
            if (sprite0_hit) dado |= 0x40;

            vblank_ativo = false;
            latch = false; // Latch reseta aqui!
            break;
        case 0x2004: // OAMDATA
            dado = oam[oam_addr];
            break;
        case 0x2007: {
            uint16_t endereco_atual = vram_addr;
            if (endereco_atual >= 0x3F00 && endereco_atual <= 0x3FFF) {
                dado = LerMemoriaPPU(endereco_atual);
                leitura_buffer = LerMemoriaPPU(endereco_atual - 0x1000);
            } else {
                dado = leitura_buffer;
                leitura_buffer = LerMemoriaPPU(endereco_atual);
            }

            if (ppu_ctrl & 0x04) vram_addr += 32;
            else                 vram_addr += 1;
            break;
        }
    }
    return dado;
}

void PPU::EscreverRegistrador(uint16_t endereco, uint8_t valor) {
    switch (endereco) {
        case 0x2000:
            ppu_ctrl = valor;
            break;
        case 0x2003:
            oam_addr = valor;
            break;
        case 0x2004:
            oam[oam_addr++] = valor;
            break;
        case 0x2005:
            if (latch == false) {
                ppu_scroll_x = valor;
                latch = true;
            } else {
                ppu_scroll_y = valor;
                latch = false;
            }
            break;
        case 0x2006:
            if (latch == false) {
                vram_addr = (uint16_t)((valor & 0x3F) << 8) | (vram_addr & 0x00FF);
                latch = true;
            } else {
                vram_addr = (vram_addr & 0xFF00) | valor;
                latch = false;

                // --- O HACK DOURADO ---
                // O Super Mario Bros usa o registrador $2006 para alinhar a câmera do HUD.
                // Isso força nossa PPU a aceitar a troca de Nametable sem piscar!
                ppu_ctrl = (ppu_ctrl & 0xFC) | ((vram_addr >> 10) & 0x03);
                ppu_scroll_y = ((vram_addr >> 5) & 0x1F) * 8;
                ppu_scroll_x = (vram_addr & 0x1F) * 8;
            }
            break;
        case 0x2007:
            EscreverMemoriaPPU(vram_addr, valor);
            if (ppu_ctrl & 0x04) vram_addr += 32;
            else                 vram_addr += 1;
            break;
    }
}