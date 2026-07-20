#define SDL_MAIN_HANDLED
#include "cpu.h"
#include "bus.h"
#include "gui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <SDL2/SDL.h>

// --- NOVO: A THREAD DE ÁUDIO DO SDL2 ---
// O SDL2 invoca esta função freneticamente num núcleo separado.
void CallbackDeAudio(void* userdata, Uint8* stream, int len) {
    APU* apu = (APU*)userdata;
    float* buffer = (float*)stream;
    int samples = len / sizeof(float);

    for (int i = 0; i < samples; i++) {
        buffer[i] = apu->ConsumirAmostra();
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) return 1;

    SDL_Window* janela = SDL_CreateWindow("Emulador NES - Projeto Sentinela",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          256 * 3, 240 * 3, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderizador = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderizador, 256, 240);
    SDL_Texture* textura = SDL_CreateTexture(renderizador, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING, 256, 240);

    Bus barramento;
    CPU minhaCpu(&barramento);

    GUI interfaceGrafica;
    interfaceGrafica.Inicializar(janela, renderizador, &minhaCpu);

    // --- CONFIGURAÇÃO DA PLACA DE SOM (Pull Mode / Callback) ---
    SDL_AudioSpec especificacoes_audio;
    especificacoes_audio.freq = 44100;
    especificacoes_audio.format = AUDIO_F32SYS;
    especificacoes_audio.channels = 1;
    especificacoes_audio.samples = 1024;
    especificacoes_audio.callback = CallbackDeAudio; // HABILITAMOS O CALLBACK
    especificacoes_audio.userdata = &barramento.apu; // Enviamos a APU para a Thread

    SDL_AudioDeviceID dispositivo_audio = SDL_OpenAudioDevice(nullptr, 0, &especificacoes_audio, nullptr, 0);
    if (dispositivo_audio == 0) {
        std::cerr << "Falha ao inicializar o áudio: " << SDL_GetError() << "\n";
    } else {
        SDL_PauseAudioDevice(dispositivo_audio, 0);
    }

    std::ifstream arquivo("mario.nes", std::ios::binary);
    if (!arquivo) {
        std::cout << "Erro: Nao foi possivel abrir mario.nes\n";
        return 1;
    }

    unsigned char header[16] = {};
    arquivo.read(reinterpret_cast<char*>(header), 16);
    if (arquivo.gcount() != 16 || std::memcmp(header, "NES\x1A", 4) != 0) {
        std::cout << "ROM iNES invalida.\n";
        return 1;
    }

    uint8_t prg_banks = header[4];
    uint8_t chr_banks = header[5];
    bool vertical_mirroring = (header[6] & 0x01) != 0;
    bool has_trainer = (header[6] & 0x04) != 0;

    barramento.prg_rom_tamanho = static_cast<uint16_t>(prg_banks) * 16384;
    barramento.chr_rom_tamanho = static_cast<uint16_t>(chr_banks) * 8192;
    barramento.espelhamento_vertical = vertical_mirroring;
    barramento.ppu.DefinirEspelhamentoVertical(vertical_mirroring);

    if (has_trainer) arquivo.seekg(512, std::ios::cur);
    if (barramento.prg_rom_tamanho > sizeof(barramento.cartucho)) barramento.prg_rom_tamanho = sizeof(barramento.cartucho);
    arquivo.read(reinterpret_cast<char*>(barramento.cartucho), barramento.prg_rom_tamanho);

    if (barramento.chr_rom_tamanho > sizeof(barramento.ppu.chr_rom)) barramento.chr_rom_tamanho = sizeof(barramento.ppu.chr_rom);
    arquivo.read(reinterpret_cast<char*>(barramento.ppu.chr_rom), barramento.chr_rom_tamanho);

    minhaCpu.Reset();
    bool rodando = true;
    SDL_Event evento;
    const Uint8* estadoTeclado = SDL_GetKeyboardState(NULL);

    Uint64 frequencia = SDL_GetPerformanceFrequency();
    const double tempo_alvo_segundos = 1.0 / 60.0988;
    Uint64 inicio_do_frame = SDL_GetPerformanceCounter();

    // --- CONTADOR DE ÁUDIO INTEIRO ---
    int relogio_audio = 0;

    while (rodando) {
        while (SDL_PollEvent(&evento)) {
            ImGui_ImplSDL2_ProcessEvent(&evento);
            if (evento.type == SDL_QUIT) rodando = false;
        }

        ImGuiIO& io = ImGui::GetIO();
        bool gui_capturou_mouse = io.WantCaptureMouse;
        bool gui_capturou_teclado = io.WantCaptureKeyboard;

        if (!interfaceGrafica.pausado) {
            int ciclos_cpu = 0;

            if (barramento.oam_dma_ciclos > 0) {
                barramento.oam_dma_ciclos--;
                ciclos_cpu = 1;
            } else {
                ciclos_cpu = minhaCpu.ExecutarCiclo();
            }

            for (int i = 0; i < ciclos_cpu; i++) {
                barramento.apu.Clock();

                // A CPU não bloqueia mais. Ela apenas despeja as amostras no Buffer.
                relogio_audio += 44100;
                if (relogio_audio >= 1789773) {
                    relogio_audio -= 1789773;

                    // O CPU aguarda microsegundos se a thread do SDL2 estiver lenta
                    // Isso amarra o emulador ao tempo físico da placa de som.
                    while (barramento.apu.BufferCheio()) {
                       // Spinlock frouxo. Evita decolar a 500 FPS.
                    }
                    barramento.apu.ArmazenarAmostra();
                }
            }

            for (int i = 0; i < ciclos_cpu * 3; i++) {
                bool frame_pronto = barramento.ppu.Clock();

                if (barramento.ppu.nmi_ocorreu) {
                    if (barramento.ppu.ppu_ctrl & 0x80) minhaCpu.NMI();
                    barramento.ppu.nmi_ocorreu = false;
                }

                if (frame_pronto) {
                    if (!gui_capturou_teclado) {
                        barramento.estado_botoes = 0;
                        if (estadoTeclado[SDL_SCANCODE_X])      barramento.estado_botoes |= (1 << 7);
                        if (estadoTeclado[SDL_SCANCODE_Z])      barramento.estado_botoes |= (1 << 6);
                        if (estadoTeclado[SDL_SCANCODE_SPACE])  barramento.estado_botoes |= (1 << 5);
                        if (estadoTeclado[SDL_SCANCODE_RETURN]) barramento.estado_botoes |= (1 << 4);
                        if (estadoTeclado[SDL_SCANCODE_UP])     barramento.estado_botoes |= (1 << 3);
                        if (estadoTeclado[SDL_SCANCODE_DOWN])   barramento.estado_botoes |= (1 << 2);
                        if (estadoTeclado[SDL_SCANCODE_LEFT])   barramento.estado_botoes |= (1 << 1);
                        if (estadoTeclado[SDL_SCANCODE_RIGHT])  barramento.estado_botoes |= (1 << 0);
                    }

                    barramento.ppu.Renderizar(textura);

                    SDL_RenderSetLogicalSize(renderizador, 0, 0);
                    SDL_RenderClear(renderizador);

                    SDL_Rect dest_rect;
                    int win_w, win_h;
                    SDL_GetWindowSize(janela, &win_w, &win_h);

                    float scale_x = (float)win_w / 256.0f;
                    float scale_y = (float)win_h / 240.0f;
                    float scale = (scale_x < scale_y) ? scale_x : scale_y;

                    dest_rect.w = 256 * scale;
                    dest_rect.h = 240 * scale;
                    dest_rect.x = (win_w - dest_rect.w) / 2;
                    dest_rect.y = (win_h - dest_rect.h) / 2;

                    SDL_RenderCopy(renderizador, textura, NULL, &dest_rect);
                    interfaceGrafica.NovaCamada();
                    interfaceGrafica.DesenharJanelas();
                    interfaceGrafica.Renderizar();
                    SDL_RenderPresent(renderizador);
                    SDL_RenderSetLogicalSize(renderizador, 256, 240);

                    // --- AUDIO-SYNC ---
                    // Como a CPU agora trava no BufferCheio() da APU, não precisamos mais do
                    // Hybrid Sleep complexo! O FPS do emulador é fisicamente cravado
                    // na velocidade em que o SDL2 esvazia as 44100 amostras do buffer.
                }
            }
        } else {
            SDL_RenderSetLogicalSize(renderizador, 0, 0);
            SDL_RenderClear(renderizador);

            SDL_Rect dest_rect;
            int win_w, win_h;
            SDL_GetWindowSize(janela, &win_w, &win_h);
            float scale_x = (float)win_w / 256.0f;
            float scale_y = (float)win_h / 240.0f;
            float scale = (scale_x < scale_y) ? scale_x : scale_y;
            dest_rect.w = 256 * scale;
            dest_rect.h = 240 * scale;
            dest_rect.x = (win_w - dest_rect.w) / 2;
            dest_rect.y = (win_h - dest_rect.h) / 2;

            SDL_RenderCopy(renderizador, textura, NULL, &dest_rect);
            interfaceGrafica.NovaCamada();
            interfaceGrafica.DesenharJanelas();
            interfaceGrafica.Renderizar();
            SDL_RenderPresent(renderizador);
            SDL_RenderSetLogicalSize(renderizador, 256, 240);
        }
    }

    interfaceGrafica.Encerrar();
    SDL_CloseAudioDevice(dispositivo_audio);
    SDL_DestroyTexture(textura);
    SDL_DestroyRenderer(renderizador);
    SDL_DestroyWindow(janela);
    SDL_Quit();
    return 0;
}