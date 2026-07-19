#include "apu.h"

// Tabela dos 4 timbres (Duty Cycles) do NES.
// Cada onda é desenhada em 8 passos. (1 = Som ligado, 0 = Som desligado)
static const uint8_t DUTY_TABLE[4][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0}, // 12.5% (Som metálico e fino)
    {0, 1, 1, 0, 0, 0, 0, 0}, // 25%   (Som clássico de melodia 8-bit)
    {0, 1, 1, 1, 1, 0, 0, 0}, // 50%   (Som anasalado, oco)
    {1, 0, 0, 1, 1, 1, 1, 1}  // 25% invertido
};

APU::APU() {
    ciclos_totais = 0;

    pulse1_habilitado = false;
    pulse1_duty = 0;
    pulse1_volume = 0;
    pulse1_timer = 0;
    pulse1_contador = 0;
    pulse1_sequenciador = 0;
}

void APU::Escrever(uint16_t endereco, uint8_t valor) {
    switch (endereco) {
        case 0x4000: // Configuração de Duty e Volume
            pulse1_duty = (valor >> 6) & 0x03; // Pega os 2 últimos bits
            pulse1_volume = valor & 0x0F;      // Pega os 4 primeiros bits
            break;

        case 0x4002: // Timer Low (Baixa Afinação)
            pulse1_timer = (pulse1_timer & 0xFF00) | valor;
            break;

        case 0x4003: // Timer High (Alta Afinação)
            pulse1_timer = (pulse1_timer & 0x00FF) | ((valor & 0x07) << 8);
            pulse1_sequenciador = 0; // Quando muda a nota, reinicia a fase da onda
            break;

        case 0x4015: // Chave Geral da APU
            pulse1_habilitado = (valor & 0x01) != 0;
            break;
    }
}

uint8_t APU::Ler(uint16_t endereco) {
    // A leitura de registradores da APU é bem específica,
    // mas retornaremos 0 por enquanto para simplificar o Pulse 1.
    return 0;
}

void APU::Clock() {
    // O timer da APU só decresce 1 vez a cada 2 ciclos da CPU!
    if (ciclos_totais % 2 == 0) {
        if (pulse1_contador > 0) {
            pulse1_contador--;
        } else {
            // Quando o timer zera, ele recarrega a afinação e dá 1 passo na onda
            pulse1_contador = pulse1_timer;
            pulse1_sequenciador = (pulse1_sequenciador + 1) % 8;
        }
    }
    ciclos_totais++;
}

double APU::ObterAmostra() {
    // Se o canal estiver mutado, ou a frequência for muito alta (timer < 8), sai silêncio.
    if (!pulse1_habilitado || pulse1_timer < 8) {
        return 0.0;
    }

    // Consulta a matriz para saber se o alto-falante deve estar "empurrado" ou "puxado"
    uint8_t estado_onda = DUTY_TABLE[pulse1_duty][pulse1_sequenciador];

    // Converte o volume do NES (0 a 15) para uma força de áudio normalizada (0.0 a 1.0)
    if (estado_onda == 1) {
        return (double)pulse1_volume / 15.0;
    } else {
        return 0.0; // Silêncio
    }
}