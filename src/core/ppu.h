#pragma once
#include <cstdint>

struct SDL_Renderer;
struct SDL_Texture; // <-- NOVO

class PPU {
public:
    PPU();

    bool Clock();
    void Renderizar(SDL_Texture* textura) const;

    uint8_t ppu_scroll_x;
    uint8_t ppu_scroll_y;
    uint8_t oam_addr;

    uint8_t chr_rom[8192];

    uint8_t vram[2048];
    uint8_t paleta[32];
    uint8_t oam[256];

    uint8_t render_scroll_x;
    uint8_t render_scroll_y;
    uint8_t render_ctrl;

    uint8_t tela_pixels[256 * 240];

    uint8_t LerRegistrador(uint16_t endereco);
    void EscreverRegistrador(uint16_t endereco, uint8_t valor);
    void DefinirEspelhamentoVertical(bool vertical);

    bool nmi_ocorreu;
    bool vblank_ativo;
    uint8_t ppu_ctrl;

    uint16_t vram_addr;
    bool latch;

    int scanline;
    int ciclo;
    bool sprite0_hit;
    mutable bool bg_transparente;
    bool espelhamento_vertical;

private:
    uint8_t CorBackgroundEmPixel(int pixel_x, int pixel_y) const;
    uint8_t CorSpriteEmPixel(int pixel_x, int pixel_y, int indice_sprite) const;
    uint8_t CorPaleta(uint8_t indice) const;
    uint8_t LerChrRomSeguro(uint16_t endereco) const;
    uint8_t LerVramSeguro(uint16_t endereco) const;
    uint8_t LerMemoriaPPU(uint16_t endereco) const;
    void EscreverMemoriaPPU(uint16_t endereco, uint8_t valor);
    uint16_t MapearNametable(uint16_t endereco) const;
    uint16_t MapearPaleta(uint16_t endereco) const;

    uint8_t leitura_buffer;
};