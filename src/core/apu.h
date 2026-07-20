#pragma once
#include <cstdint>
#include <mutex>
#include <vector>

class APU {
public:
    APU();
    void Escrever(uint16_t endereco, uint8_t valor);
    uint8_t Ler(uint16_t endereco);
    void Clock();

    // --- NOVO: Arquitetura Thread-Safe ---
    double ObterAmostra(); // Calcula a amostra atual
    void ArmazenarAmostra(); // Guarda no Ring Buffer (chamado pela CPU)
    float ConsumirAmostra(); // Retira do Ring Buffer (chamado pelo SDL2 via Callback)
    bool BufferCheio(); // Informa se o buffer atingiu o limite de segurança

private:
    uint32_t ciclos_totais;

    // --- Ring Buffer de Áudio ---
    std::vector<float> ring_buffer;
    size_t rb_escrita;
    size_t rb_leitura;
    std::mutex mtx_audio;

    // --- Maestro (Frame Sequencer) ---
    uint32_t frame_counter;
    uint8_t frame_sequencer_passo;

    // --- Canal 1: Pulse 1 ---
    bool pulse1_habilitado;
    uint8_t pulse1_duty;
    bool pulse1_env_loop, pulse1_env_const;
    uint8_t pulse1_env_param, pulse1_env_decai, pulse1_env_div;
    bool pulse1_env_iniciar;
    bool pulse1_sweep_hab, pulse1_sweep_neg, pulse1_sweep_reload;
    uint8_t pulse1_sweep_div, pulse1_sweep_param, pulse1_sweep_shift;
    uint16_t pulse1_timer, pulse1_contador;
    uint8_t pulse1_sequenciador;

    // --- Canal 2: Pulse 2 (Clone do Pulse 1) ---
    bool pulse2_habilitado;
    uint8_t pulse2_duty;
    bool pulse2_env_loop, pulse2_env_const;
    uint8_t pulse2_env_param, pulse2_env_decai, pulse2_env_div;
    bool pulse2_env_iniciar;
    bool pulse2_sweep_hab, pulse2_sweep_neg, pulse2_sweep_reload;
    uint8_t pulse2_sweep_div, pulse2_sweep_param, pulse2_sweep_shift;
    uint16_t pulse2_timer, pulse2_contador;
    uint8_t pulse2_sequenciador;

    // --- Canal 3: Triangle (Grave) ---
    bool triangle_habilitado;
    uint16_t triangle_timer, triangle_contador;
    uint8_t triangle_sequenciador;
    uint8_t triangle_linear_counter, triangle_linear_reload_val;
    bool triangle_linear_reload_flag;
    bool triangle_control_flag;

    // --- Canal 4: Noise (Bateria/Explosão) ---
    bool noise_habilitado;
    bool noise_env_loop, noise_env_const;
    uint8_t noise_env_param, noise_env_decai, noise_env_div;
    bool noise_env_iniciar;
    uint16_t noise_timer, noise_contador;
    uint16_t noise_shift_reg;
    bool noise_modo;
};