#include "apu.h"

static const uint8_t DUTY_TABLE[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1}
};

static const uint8_t TRIANGLE_TABLE[32] = {
    15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const uint16_t NOISE_TABLE[16] = {
    4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
};

APU::APU() : ring_buffer(4096, 0.0f), rb_escrita(0), rb_leitura(0) {
    ciclos_totais = 0; frame_counter = 0; frame_sequencer_passo = 0;

    pulse1_habilitado = false; pulse1_duty = 0; pulse1_timer = 0; pulse1_contador = 0; pulse1_sequenciador = 0;
    pulse2_habilitado = false; pulse2_duty = 0; pulse2_timer = 0; pulse2_contador = 0; pulse2_sequenciador = 0;

    triangle_habilitado = false; triangle_timer = 0; triangle_contador = 0; triangle_sequenciador = 0;
    triangle_linear_counter = 0; triangle_linear_reload_val = 0; triangle_linear_reload_flag = false;

    noise_habilitado = false; noise_timer = 0; noise_contador = 0; noise_shift_reg = 1; noise_modo = false;
}

void APU::Escrever(uint16_t endereco, uint8_t valor) {
    switch (endereco) {
        // --- PULSE 1 ---
        case 0x4000:
            pulse1_duty = (valor >> 6) & 0x03; pulse1_env_loop = (valor & 0x20);
            pulse1_env_const = (valor & 0x10); pulse1_env_param = valor & 0x0F; break;
        case 0x4001:
            pulse1_sweep_hab = (valor & 0x80); pulse1_sweep_param = ((valor >> 4) & 0x07);
            pulse1_sweep_neg = (valor & 0x08); pulse1_sweep_shift = valor & 0x07; pulse1_sweep_reload = true; break;
        case 0x4002: pulse1_timer = (pulse1_timer & 0xFF00) | valor; break;
        case 0x4003:
            pulse1_timer = (pulse1_timer & 0x00FF) | ((valor & 0x07) << 8);
            pulse1_sequenciador = 0; pulse1_env_iniciar = true; break;

        // --- PULSE 2 ---
        case 0x4004:
            pulse2_duty = (valor >> 6) & 0x03; pulse2_env_loop = (valor & 0x20);
            pulse2_env_const = (valor & 0x10); pulse2_env_param = valor & 0x0F; break;
        case 0x4005:
            pulse2_sweep_hab = (valor & 0x80); pulse2_sweep_param = ((valor >> 4) & 0x07);
            pulse2_sweep_neg = (valor & 0x08); pulse2_sweep_shift = valor & 0x07; pulse2_sweep_reload = true; break;
        case 0x4006: pulse2_timer = (pulse2_timer & 0xFF00) | valor; break;
        case 0x4007:
            pulse2_timer = (pulse2_timer & 0x00FF) | ((valor & 0x07) << 8);
            pulse2_sequenciador = 0; pulse2_env_iniciar = true; break;

        // --- TRIANGLE ---
        case 0x4008:
            triangle_control_flag = (valor & 0x80);
            triangle_linear_reload_val = valor & 0x7F; break;
        case 0x400A: triangle_timer = (triangle_timer & 0xFF00) | valor; break;
        case 0x400B:
            triangle_timer = (triangle_timer & 0x00FF) | ((valor & 0x07) << 8);
            triangle_linear_reload_flag = true; break;

        // --- NOISE ---
        case 0x400C:
            noise_env_loop = (valor & 0x20); noise_env_const = (valor & 0x10);
            noise_env_param = valor & 0x0F; break;
        case 0x400E:
            noise_modo = (valor & 0x80);
            noise_timer = NOISE_TABLE[valor & 0x0F]; break;
        case 0x400F:
            noise_env_iniciar = true; break;

        // --- CHAVE GERAL ---
        case 0x4015:
            pulse1_habilitado = (valor & 0x01); pulse2_habilitado = (valor & 0x02);
            triangle_habilitado = (valor & 0x04); noise_habilitado = (valor & 0x08);
            break;
    }
}

uint8_t APU::Ler(uint16_t endereco) { return 0; }

void APU::Clock() {
    // 1. O FRAME SEQUENCER (~240Hz)
    if (frame_counter > 7457) {
        frame_counter = 0;
        frame_sequencer_passo++;

        if (pulse1_env_iniciar) { pulse1_env_iniciar = false; pulse1_env_decai = 15; pulse1_env_div = pulse1_env_param; }
        else if (pulse1_env_div > 0) { pulse1_env_div--; }
        else { pulse1_env_div = pulse1_env_param; if (pulse1_env_decai > 0) pulse1_env_decai--; else if (pulse1_env_loop) pulse1_env_decai = 15; }

        if (pulse2_env_iniciar) { pulse2_env_iniciar = false; pulse2_env_decai = 15; pulse2_env_div = pulse2_env_param; }
        else if (pulse2_env_div > 0) { pulse2_env_div--; }
        else { pulse2_env_div = pulse2_env_param; if (pulse2_env_decai > 0) pulse2_env_decai--; else if (pulse2_env_loop) pulse2_env_decai = 15; }

        if (noise_env_iniciar) { noise_env_iniciar = false; noise_env_decai = 15; noise_env_div = noise_env_param; }
        else if (noise_env_div > 0) { noise_env_div--; }
        else { noise_env_div = noise_env_param; if (noise_env_decai > 0) noise_env_decai--; else if (noise_env_loop) noise_env_decai = 15; }

        if (triangle_linear_reload_flag) { triangle_linear_counter = triangle_linear_reload_val; }
        else if (triangle_linear_counter > 0) { triangle_linear_counter--; }
        if (!triangle_control_flag) triangle_linear_reload_flag = false;

        if (frame_sequencer_passo == 1 || frame_sequencer_passo == 3) {
            if (pulse1_sweep_div > 0) { pulse1_sweep_div--; }
            else {
                if (pulse1_sweep_hab && pulse1_sweep_shift > 0 && pulse1_timer >= 8) {
                    uint16_t mudanca = pulse1_timer >> pulse1_sweep_shift;
                    if (pulse1_sweep_neg) pulse1_timer = pulse1_timer - mudanca - 1; else pulse1_timer += mudanca;
                }
                pulse1_sweep_div = pulse1_sweep_param;
            }
            if (pulse1_sweep_reload) { pulse1_sweep_div = pulse1_sweep_param; pulse1_sweep_reload = false; }

            if (pulse2_sweep_div > 0) { pulse2_sweep_div--; }
            else {
                if (pulse2_sweep_hab && pulse2_sweep_shift > 0 && pulse2_timer >= 8) {
                    uint16_t mudanca = pulse2_timer >> pulse2_sweep_shift;
                    if (pulse2_sweep_neg) pulse2_timer = pulse2_timer - mudanca; else pulse2_timer += mudanca;
                }
                pulse2_sweep_div = pulse2_sweep_param;
            }
            if (pulse2_sweep_reload) { pulse2_sweep_div = pulse2_sweep_param; pulse2_sweep_reload = false; }
        }

        if (frame_sequencer_passo >= 4) frame_sequencer_passo = 0;
    }
    frame_counter++;

    if (triangle_habilitado && triangle_linear_counter > 0 && triangle_timer >= 2) {
        if (triangle_contador > 0) { triangle_contador--; }
        else {
            triangle_contador = triangle_timer;
            triangle_sequenciador = (triangle_sequenciador + 1) % 32;
        }
    }

    if (ciclos_totais % 2 == 0) {
        if (pulse1_contador > 0) { pulse1_contador--; }
        else { pulse1_contador = pulse1_timer; pulse1_sequenciador = (pulse1_sequenciador + 1) % 8; }

        if (pulse2_contador > 0) { pulse2_contador--; }
        else { pulse2_contador = pulse2_timer; pulse2_sequenciador = (pulse2_sequenciador + 1) % 8; }

        if (noise_contador > 0) { noise_contador--; }
        else {
            noise_contador = noise_timer;
            uint16_t feedback = (noise_shift_reg & 1) ^ ((noise_shift_reg >> (noise_modo ? 6 : 1)) & 1);
            noise_shift_reg >>= 1;
            noise_shift_reg |= (feedback << 14);
        }
    }
    ciclos_totais++;
}

double APU::ObterAmostra() {
    double p1 = 0.0, p2 = 0.0, tri = 0.0, noi = 0.0;

    if (pulse1_habilitado && pulse1_timer >= 8 && pulse1_timer <= 0x7FF && DUTY_TABLE[pulse1_duty][pulse1_sequenciador]) {
        p1 = (double)(pulse1_env_const ? pulse1_env_param : pulse1_env_decai) / 15.0;
    }

    if (pulse2_habilitado && pulse2_timer >= 8 && pulse2_timer <= 0x7FF && DUTY_TABLE[pulse2_duty][pulse2_sequenciador]) {
        p2 = (double)(pulse2_env_const ? pulse2_env_param : pulse2_env_decai) / 15.0;
    }

    if (triangle_habilitado) {
        tri = (double)TRIANGLE_TABLE[triangle_sequenciador] / 15.0;
    }

    if (noise_habilitado && (noise_shift_reg & 1) == 0) {
        noi = (double)(noise_env_const ? noise_env_param : noise_env_decai) / 15.0;
    }

    // Mixagem simplificada
    return (p1 * 0.25) + (p2 * 0.25) + (tri * 0.35) + (noi * 0.15);
}

bool APU::BufferCheio() {
    std::lock_guard<std::mutex> lock(mtx_audio);
    // Se a distância entre escrita e leitura for muito grande, o buffer está "cheio" na perspectiva da CPU
    size_t ocupado = (rb_escrita - rb_leitura + ring_buffer.size()) % ring_buffer.size();
    return ocupado >= (ring_buffer.size() - 256); // Retorna true se estiver quase estourando
}

void APU::ArmazenarAmostra() {
    // É chamado pela CPU no main.cpp
    float amostra = (float)ObterAmostra() * 0.3f; // Reduz o ganho total para evitar saturação

    std::lock_guard<std::mutex> lock(mtx_audio);
    ring_buffer[rb_escrita] = amostra;
    rb_escrita = (rb_escrita + 1) % ring_buffer.size();
}

float APU::ConsumirAmostra() {
    // É chamado freneticamente pela Thread de Áudio do SDL2
    std::lock_guard<std::mutex> lock(mtx_audio);
    if (rb_escrita == rb_leitura) return 0.0f; // Se a CPU não entregou rápido o suficiente, toca silêncio

    float amostra = ring_buffer[rb_leitura];
    rb_leitura = (rb_leitura + 1) % ring_buffer.size();
    return amostra;
}