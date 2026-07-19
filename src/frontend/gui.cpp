#include "gui.h"
#include "cpu.h" // Incluímos o header da CPU aqui no .cpp!
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <cstdlib>

void GUI::Inicializar(SDL_Window* janela, SDL_Renderer* renderizador, CPU* cpu) {
    meu_renderizador = renderizador;
    cpu_ref = cpu; // Guardamos o ponteiro da CPU

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(janela, renderizador);
    ImGui_ImplSDLRenderer2_Init(renderizador);
}

void GUI::ProcessarEvento(const SDL_Event* evento) {
    ImGui_ImplSDL2_ProcessEvent(evento);
}

void GUI::NovaCamada() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void GUI::DesenharJanelas() {
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("Arquivo")) {
            if (ImGui::MenuItem("Sair", "Alt+F4")) { exit(0); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Emulacao")) {
            ImGui::MenuItem("Pausar", "P", &pausado);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Ferramentas")) {
            ImGui::MenuItem("Raio-X da CPU", "F12", &mostrar_debug);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (mostrar_debug && cpu_ref != nullptr) {
        ImGui::Begin("Raio-X da CPU (MOS 6502)", &mostrar_debug, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Desempenho: %.1f FPS", ImGui::GetIO().Framerate);
        ImGui::Separator();

        // Exibição dos Registradores em Hexadecimal
        ImGui::Text("Registradores:");
        ImGui::Text("PC: 0x%04X", cpu_ref->PC);
        ImGui::Text("A:  0x%02X", cpu_ref->A);
        ImGui::Text("X:  0x%02X", cpu_ref->X);
        ImGui::Text("Y:  0x%02X", cpu_ref->Y);
        ImGui::Text("SP: 0x%02X", cpu_ref->SP);

        ImGui::Separator();

        // Status Flags desmontadas bit a bit
        ImGui::Text("Flags (Status): 0x%02X", cpu_ref->Status);
        ImGui::Text("N  V  U  B  D  I  Z  C"); // Letras das Flags
        ImGui::Text("%d  %d  %d  %d  %d  %d  %d  %d",
            (cpu_ref->Status >> 7) & 1, // Negativo
            (cpu_ref->Status >> 6) & 1, // Overflow
            (cpu_ref->Status >> 5) & 1, // Unused
            (cpu_ref->Status >> 4) & 1, // Break
            (cpu_ref->Status >> 3) & 1, // Decimal
            (cpu_ref->Status >> 2) & 1, // Interrupt Disable
            (cpu_ref->Status >> 1) & 1, // Zero
            (cpu_ref->Status >> 0) & 1  // Carry
        );

        ImGui::Separator();
        ImGui::Checkbox("Pausar Emulador", &pausado);
        ImGui::End();
    }
}

void GUI::Renderizar() {
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), meu_renderizador);
}

void GUI::Encerrar() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}