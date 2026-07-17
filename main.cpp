#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <fstream>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

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
    while (rodando) {
        // Escuta os eventos do Windows (Ex: Clicar no botão X para fechar a janela)
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                rodando = false;
            }
        }

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

            // --- DESENHAR O NAMETABLE (CENÁRIO) ---
            // Limpa a tela com fundo preto
            SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
            SDL_RenderClear(renderizador);

            // A tela do NES é uma grade de 32 colunas por 30 linhas (960 blocos no total)
            for (int y = 0; y < 30; y++) {
                for (int x = 0; x < 32; x++) {
                    // O Nametable 0 fica no início da nossa VRAM (índice 0 ao 959)
                    int vram_index = (y * 32) + x;

                    // Lemos a ID da peça que o jogo quer que seja desenhada aqui
                    uint8_t tile_id = barramento.ppu.vram[vram_index];

                    // O Mario escolhe se o cenário usa a Tabela da Esquerda (0) ou Direita (4096)
                    // Ele diz isso através do bit 4 do registrador PPUCTRL
                    uint16_t padrao_banco = (barramento.ppu.ppu_ctrl & 0x10) ? 4096 : 0;
                    uint16_t tile_endereco = padrao_banco + (tile_id * 16);

                    // Desenha os 8x8 pixels dessa peça
                    for (int linha = 0; linha < 8; linha++) {
                        uint8_t byte1 = barramento.ppu.chr_rom[tile_endereco + linha];
                        uint8_t byte2 = barramento.ppu.chr_rom[tile_endereco + linha + 8];

                        for (int bit = 7; bit >= 0; bit--) {
                            uint8_t color_bit1 = (byte1 >> bit) & 1;
                            uint8_t color_bit2 = (byte2 >> bit) & 1;
                            uint8_t color_val = (color_bit2 << 1) | color_bit1;

                            // Cores temporárias (em tons de cinza) até implementarmos as Paletas reais
                            if (color_val == 0) SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
                            if (color_val == 1) SDL_SetRenderDrawColor(renderizador, 85, 85, 85, 255);
                            if (color_val == 2) SDL_SetRenderDrawColor(renderizador, 170, 170, 170, 255);
                            if (color_val == 3) SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255);

                            // Calcula a coordenada exata na tela inteira
                            int pixel_x = (x * 8) + (7 - bit);
                            int pixel_y = (y * 8) + linha;

                            // Se a cor for 0 (transparente), o NES desenha a cor de fundo (preto por enquanto)
                            if (color_val != 0) {
                                SDL_RenderDrawPoint(renderizador, pixel_x, pixel_y);
                            }
                        }
                    }
                }
            }

            // O NES possui 512 blocos gráficos (Tiles) gravados no cartucho, cada um com 8x8 pixels.
            for (int tile = 0; tile < 512; tile++) {
                int tile_x = (tile % 32) * 8; // Distribui desenhando 32 blocos por linha
                int tile_y = (tile / 32) * 8; // Cria 16 linhas horizontais preenchidas

                // Cada bloco é composto de 8 linhas horizontais de pixels
                for (int linha = 0; linha < 8; linha++) {
                    // No NES, as cores de um bloco são formadas unindo bits de dois bytes separados na memória
                    uint8_t byte1 = barramento.ppu.chr_rom[tile * 16 + linha];
                    uint8_t byte2 = barramento.ppu.chr_rom[tile * 16 + linha + 8];

                    // Varremos os 8 pixels de cada linha da direita para a esquerda (do bit 7 ao 0)
                    for (int bit = 7; bit >= 0; bit--) {
                        uint8_t color_bit1 = (byte1 >> bit) & 1;
                        uint8_t color_bit2 = (byte2 >> bit) & 1;

                        // Juntamos as duas fatias de bits para gerar um ID de cor variando de 0 a 3
                        uint8_t color_val = (color_bit2 << 1) | color_bit1;

                        // Como ainda não temos paletas, traduzimos os IDs temporariamente para tons de cinza
                        if (color_val == 0) SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);       // Preto/Transparente
                        if (color_val == 1) SDL_SetRenderDrawColor(renderizador, 85, 85, 85, 255);    // Cinza Escuro
                        if (color_val == 2) SDL_SetRenderDrawColor(renderizador, 170, 170, 170, 255); // Cinza Claro
                        if (color_val == 3) SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 255); // Branco

                        int pixel_x = tile_x + (7 - bit);
                        int pixel_y = tile_y + linha;

                        // Se o pixel não for transparente (0), carimba ele na memória da placa de vídeo
                        if (color_val != 0) {
                            SDL_RenderDrawPoint(renderizador, pixel_x, pixel_y);
                        }
                    }
                }
            }

            // Dá o comando para a placa de vídeo pegar tudo e finalmente jogar na tela do usuário
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