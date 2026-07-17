#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <fstream>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

// Paleta de cores oficial do NES traduzida para RGB (Formato: R, G, B)
struct RGB { uint8_t r, g, b; };
const RGB NES_PALETTE[64] = {
    {84, 84, 84}, {0, 30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0},
    {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {152, 150, 152}, {8, 76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0},
    {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
    {236, 238, 236}, {76, 154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32},
    {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},
    {236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144},
    {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
};

int main(int argc, char* argv[]) {
    // --- 1. INICIALIZAÇÃO DA SDL2 ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "ERRO SDL: " << SDL_GetError() << "\n";
        return 1;
    }

    // Cria a Janela (Tamanho original do NES é 256x240, multiplicamos por 3 para ficar visível)
    SDL_Window* janela = SDL_CreateWindow("Meu Emulador de NES - Projeto Sentinela",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          256 * 3, 240 * 3, 0);

    // Cria o Renderizador (A Placa de Vídeo acelerada por Hardware)
    SDL_Renderer* renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);

    // Escala tudo em 3x para que o NES de 256x240 preencha a nossa janela de forma nativa
    SDL_RenderSetScale(renderizador, 3.0f, 3.0f);

    // Pinta a tela de preto para começar
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);
    SDL_RenderPresent(renderizador);


    // --- 2. INICIALIZAÇÃO DO HARDWARE (Nosso Emulador) ---
    Bus barramento;
    CPU minhaCpu(&barramento);

    // Abre o arquivo do jogo (A ROM do Super Mario Bros)
    std::ifstream arquivo("mario.nes", std::ios::binary);
    if (!arquivo) {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo mario.nes!\n";
        return 1;
    }

    // Pula o cabeçalho iNES (16 bytes) e carrega a PRG-ROM (O código-fonte de 32KB)
    arquivo.seekg(16, std::ios::beg);
    arquivo.read(reinterpret_cast<char*>(barramento.cartucho), 32768);
    std::cout << "[Cartucho] PRG-ROM (Codigo) carregada com sucesso!\n";

    // Lê a CHR-ROM (Gráficos da PPU de 8KB) exatamente após o código
    arquivo.seekg(16 + 32768, std::ios::beg);
    arquivo.read(reinterpret_cast<char*>(barramento.ppu.chr_rom), 8192);
    std::cout << "[Cartucho] CHR-ROM (Graficos) carregada com sucesso!\n";

    // Liga a CPU na tomada
    minhaCpu.Reset();
    int ciclos = 0;
    bool rodando = true;
    SDL_Event evento;

    std::cout << "--- INICIANDO O EMULADOR NES ---\n";

    // --- 3. O LOOP PRINCIPAL DO CONSOLE ---
    // Pega o acesso direto e contínuo ao estado físico de todas as teclas do seu teclado
    const Uint8* estadoTeclado = SDL_GetKeyboardState(NULL);

    while (rodando) {
        // A CPU corre processando as instruções do jogo!
        int ciclos_instrucao = minhaCpu.ExecutarCiclo();
        ciclos += ciclos_instrucao;

        // O VBlank acontece a cada 29780 ciclos (Equivale a 1 Frame gerado no monitor a 60Hz)
        if (ciclos >= 29780) {
            barramento.ppu.nmi_ocorreu = true;

            // Se a tela estiver habilitada no PPUCTRL, envia a interrupção pra CPU desenhar
            if (barramento.ppu.ppu_ctrl & 0x80) {
                minhaCpu.NMI();
            }

            ciclos -= 29780; // Zera a contagem para o próximo frame

            // --- PROCESSAMENTO DE EVENTOS (Apenas 1x por frame!) ---
            while (SDL_PollEvent(&evento)) {
                if (evento.type == SDL_QUIT) {
                    rodando = false;
                }
            }

            // --- ATUALIZAR O CONTROLE (Joypad 1) ---
            barramento.controle_estado = 0;

            if (estadoTeclado[SDL_SCANCODE_X])      barramento.controle_estado |= (1 << 7); // A
            if (estadoTeclado[SDL_SCANCODE_Z])      barramento.controle_estado |= (1 << 6); // B
            if (estadoTeclado[SDL_SCANCODE_SPACE])  barramento.controle_estado |= (1 << 5); // Select
            if (estadoTeclado[SDL_SCANCODE_RETURN]) barramento.controle_estado |= (1 << 4); // Start
            if (estadoTeclado[SDL_SCANCODE_UP])     barramento.controle_estado |= (1 << 3); // Cima
            if (estadoTeclado[SDL_SCANCODE_DOWN])   barramento.controle_estado |= (1 << 2); // Baixo
            if (estadoTeclado[SDL_SCANCODE_LEFT])   barramento.controle_estado |= (1 << 1); // Esquerda
            if (estadoTeclado[SDL_SCANCODE_RIGHT])  barramento.controle_estado |= (1 << 0); // Direita

            // --- DESENHAR O NAMETABLE E SCROLL (CENÁRIO DUPLO) ---
            SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
            SDL_RenderClear(renderizador);

            int nametable_base = barramento.ppu.ppu_ctrl & 0x03;
            int scroll_x_total = barramento.ppu.ppu_scroll_x + ((nametable_base & 1) * 256);
            int scroll_y_total = barramento.ppu.ppu_scroll_y;

            for (int nametable_offset = 0; nametable_offset < 2; nametable_offset++) {
                int vram_nametable_base = nametable_offset * 1024;

                for (int y = 0; y < 30; y++) {
                    for (int x = 0; x < 32; x++) {
                        int vram_index = vram_nametable_base + (y * 32) + x;
                        uint8_t tile_id = barramento.ppu.vram[vram_index];

                        uint16_t padrao_banco = (barramento.ppu.ppu_ctrl & 0x10) ? 4096 : 0;
                        uint16_t tile_endereco = padrao_banco + (tile_id * 16);

                        int atributo_index = vram_nametable_base + 960 + ((y / 4) * 8) + (x / 4);
                        uint8_t atributo_byte = barramento.ppu.vram[atributo_index];
                        int shift = (((y % 4) / 2) * 4) + (((x % 4) / 2) * 2);
                        uint8_t paleta_id = (atributo_byte >> shift) & 0x03;

                        for (int linha = 0; linha < 8; linha++) {
                            uint8_t byte1 = barramento.ppu.chr_rom[tile_endereco + linha];
                            uint8_t byte2 = barramento.ppu.chr_rom[tile_endereco + linha + 8];

                            for (int bit = 7; bit >= 0; bit--) {
                                uint8_t color_bit1 = (byte1 >> bit) & 1;
                                uint8_t color_bit2 = (byte2 >> bit) & 1;
                                uint8_t color_val = (color_bit2 << 1) | color_bit1;

                                uint16_t paleta_endereco = (paleta_id * 4) + color_val;
                                if (color_val == 0) paleta_endereco = 0;

                                uint8_t nes_color = barramento.ppu.paleta[paleta_endereco];
                                RGB rgb = NES_PALETTE[nes_color & 0x3F];
                                SDL_SetRenderDrawColor(renderizador, rgb.r, rgb.g, rgb.b, 255);

                                int pos_x_mundo = (nametable_offset * 256) + (x * 8) + (7 - bit);
                                int pixel_x = (pos_x_mundo - scroll_x_total + 512) % 512;
                                int pixel_y = (y * 8) + linha - scroll_y_total;

                                if (pixel_x >= 0 && pixel_x < 256 && pixel_y >= 0 && pixel_y < 240) {
                                    SDL_RenderDrawPoint(renderizador, pixel_x, pixel_y);
                                }
                            }
                        }
                    }
                }
            }

            // --- DESENHAR OS SPRITES (OAM) ---
            for (int i = 63; i >= 0; i--) {
                int oam_index = i * 4;
                uint8_t sprite_y = barramento.ppu.oam[oam_index];
                uint8_t tile_id  = barramento.ppu.oam[oam_index + 1];
                uint8_t atributo = barramento.ppu.oam[oam_index + 2];
                uint8_t sprite_x = barramento.ppu.oam[oam_index + 3];

                if (sprite_y >= 239) continue;

                uint8_t paleta_id = (atributo & 0x03) + 4;
                bool flip_h = atributo & 0x40;
                bool flip_v = atributo & 0x80;

                uint16_t padrao_banco = (barramento.ppu.ppu_ctrl & 0x08) ? 4096 : 0;
                uint16_t tile_endereco = padrao_banco + (tile_id * 16);

                for (int linha = 0; linha < 8; linha++) {
                    int linha_real = flip_v ? (7 - linha) : linha;
                    uint8_t byte1 = barramento.ppu.chr_rom[tile_endereco + linha_real];
                    uint8_t byte2 = barramento.ppu.chr_rom[tile_endereco + linha_real + 8];

                    for (int bit = 7; bit >= 0; bit--) {
                        int bit_real = flip_h ? (7 - bit) : bit;
                        uint8_t color_bit1 = (byte1 >> bit_real) & 1;
                        uint8_t color_bit2 = (byte2 >> bit_real) & 1;
                        uint8_t color_val = (color_bit2 << 1) | color_bit1;

                        if (color_val == 0) continue;

                        uint16_t paleta_endereco = (paleta_id * 4) + color_val;
                        uint8_t nes_color = barramento.ppu.paleta[paleta_endereco];
                        RGB rgb = NES_PALETTE[nes_color & 0x3F];
                        SDL_SetRenderDrawColor(renderizador, rgb.r, rgb.g, rgb.b, 255);

                        int pixel_x = sprite_x + (7 - bit);
                        int pixel_y = sprite_y + linha;

                        SDL_RenderDrawPoint(renderizador, pixel_x, pixel_y);
                    }
                }
            }

            SDL_RenderPresent(renderizador);
        }
    }

    // --- 4. DESLIGANDO A TV ---
    std::cout << "[Sistema] Encerrando emulador...\n";
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(janela);
    SDL_Quit();

    return 0;
}