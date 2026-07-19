#pragma once
#include <cstdint>

class APU {
public:
    APU();

    // Comunicação com o barramento
    void Escrever(uint16_t endereco, uint8_t valor);
    uint8_t Ler(uint16_t endereco);

    // O coração do sintetizador
    void Clock();

    // Entrega o áudio pronto para o SDL2 (um valor entre 0.0 e 1.0)
    double ObterAmostra();

private:
    uint32_t ciclos_totais;

    // --- Canal Pulse 1 (Onda Quadrada Principal) ---
    bool pulse1_habilitado;
    uint8_t pulse1_duty;        // Tipo da onda (12.5%, 25%, 50%, 75%)
    uint8_t pulse1_volume;      // Volume (0 a 15)

    uint16_t pulse1_timer;      // A afinação (nota musical) que o jogo pediu
    uint16_t pulse1_contador;   // O cronômetro interno regressivo
    uint8_t pulse1_sequenciador;// O passo atual da onda (vai de 0 a 7)
};