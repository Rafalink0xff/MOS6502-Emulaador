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

    // Pinta a tela de preto para começar
    SDL_SetRenderDrawColor(renderizador, 0, 0, 0, 255);
    SDL_RenderClear(renderizador);
    SDL_RenderPresent(renderizador);


    // --- 2. INICIALIZAÇÃO DO HARDWARE (Nosso Emulador) ---
    Bus barramento;
    CPU minhaCpu(&barramento);

    std::ifstream arquivo("mario.nes", std::ios::binary);
    if (!arquivo) {
        std::cout << "ERRO: Nao foi possivel abrir o arquivo mario.nes!\n";
        return 1;
    }

    arquivo.seekg(16, std::ios::beg);
    arquivo.read(reinterpret_cast<char*>(barramento.cartucho), 32768);
    std::cout << "[Cartucho] Jogo carregado na memoria com sucesso!\n";

    minhaCpu.Reset();
    int ciclos = 0;
    bool rodando = true;
    SDL_Event evento;

    std::cout << "--- INICIANDO O EMULADOR NES ---\n";

    // --- 3. O LOOP PRINCIPAL DO CONSOLE ---
    while (rodando) {
        // Escuta os eventos do Windows (Ex: Clicar no X para fechar a janela)
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                rodando = false;
            }
        }

        // A CPU corre!
        int ciclos_instrucao = minhaCpu.ExecutarCiclo();
        ciclos += ciclos_instrucao;

        // O VBlank acontece
        if (ciclos >= 29780) {
            barramento.ppu.nmi_ocorreu = true;

            if (barramento.ppu.ppu_ctrl & 0x80) {
                minhaCpu.NMI();
            }

            ciclos -= 29780;
        }
    }

    // --- 4. DESLIGANDO A TV ---
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(janela);
    SDL_Quit();

    return 0;
}