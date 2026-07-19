#pragma once
#include <SDL2/SDL.h>

class CPU; // "Avisa" o compilador que a classe CPU existe, sem precisar incluir o header inteiro aqui.

class GUI {
private:
    SDL_Renderer* meu_renderizador;
    CPU* cpu_ref; // <-- NOVO: O "olho" da GUI para o processador

public:
    // Atualizamos a assinatura para receber o ponteiro da CPU
    void Inicializar(SDL_Window* janela, SDL_Renderer* renderizador, CPU* cpu);
    void ProcessarEvento(const SDL_Event* evento);

    void NovaCamada();
    void DesenharJanelas();
    void Renderizar();
    void Encerrar();

    bool pausado = false;
    bool mostrar_debug = false;
};