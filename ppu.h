#pragma once
#include <cstdint>

struct SDL_Renderer;

class PPU {
public:
    PPU();

    bool Clock();
    void Renderizar(SDL_Renderer* renderer) const;

    uint8_t ppu_scroll_x;
    uint8_t ppu_scroll_y;

    uint8_t chr_rom[8192]; // Os 8KB de gráficos (Tiles e Sprites) do cartucho

    uint8_t vram[2048];
    uint8_t paleta[32];
    uint8_t oam[256];

    uint8_t LerRegistrador(uint16_t endereco);
    void EscreverRegistrador(uint16_t endereco, uint8_t valor);

    bool nmi_ocorreu;
    uint8_t ppu_ctrl; // NOVO: Guarda as configurações enviadas pelo Mario

    uint16_t vram_addr; // O endereço de 16 bits que a CPU montar
    bool latch;         // 0 = Esperando byte Alto, 1 = Esperando byte Baixo

    int scanline;
    int ciclo;
    bool sprite0_hit;

private:
    uint8_t CorBackgroundEmPixel(int pixel_x, int pixel_y) const;
    uint8_t CorSprite0EmPixel(int pixel_x, int pixel_y) const;
};