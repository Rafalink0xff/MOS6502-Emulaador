#include "cpu.h"
#include "bus.h"
#include <iostream>
#include <fstream>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window* janela = SDL_CreateWindow("Emulador NES - Projeto Sentinela",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          256 * 3, 240 * 3, 0);

    SDL_Renderer* renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderizador, 3.0f, 3.0f);

    Bus barramento;
    CPU minhaCpu(&barramento);

    std::ifstream arquivo("mario.nes", std::ios::binary);
    if (!arquivo) return 1;

    arquivo.seekg(16, std::ios::beg);
    arquivo.read(reinterpret_cast<char*>(barramento.cartucho), 32768);
    arquivo.seekg(16 + 32768, std::ios::beg);
    arquivo.read(reinterpret_cast<char*>(barramento.ppu.chr_rom), 8192);

    minhaCpu.Reset();
    bool rodando = true;
    SDL_Event evento;
    const Uint8* estadoTeclado = SDL_GetKeyboardState(NULL);

    while (rodando) {
        int ciclos_instrucao = minhaCpu.ExecutarCiclo();
        for (int i = 0; i < ciclos_instrucao * 3; i++) {
            bool frame_pronto = barramento.ppu.Clock();
            if (barramento.ppu.nmi_ocorreu) {
                if (barramento.ppu.ppu_ctrl & 0x80) minhaCpu.NMI();
                barramento.ppu.nmi_ocorreu = false;
            }

            if (frame_pronto) {
                while (SDL_PollEvent(&evento)) {
                    if (evento.type == SDL_QUIT) rodando = false;
                }

                barramento.controle_estado = 0;
                if (estadoTeclado[SDL_SCANCODE_X])      barramento.controle_estado |= (1 << 7);
                if (estadoTeclado[SDL_SCANCODE_Z])      barramento.controle_estado |= (1 << 6);
                if (estadoTeclado[SDL_SCANCODE_SPACE])  barramento.controle_estado |= (1 << 5);
                if (estadoTeclado[SDL_SCANCODE_RETURN]) barramento.controle_estado |= (1 << 4);
                if (estadoTeclado[SDL_SCANCODE_UP])     barramento.controle_estado |= (1 << 3);
                if (estadoTeclado[SDL_SCANCODE_DOWN])   barramento.controle_estado |= (1 << 2);
                if (estadoTeclado[SDL_SCANCODE_LEFT])   barramento.controle_estado |= (1 << 1);
                if (estadoTeclado[SDL_SCANCODE_RIGHT])  barramento.controle_estado |= (1 << 0);

                barramento.ppu.Renderizar(renderizador);
            }
        }
    }
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(janela);
    SDL_Quit();
    return 0;
}