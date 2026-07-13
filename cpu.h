#pragma once 
#include <cstdint> 
#include <string> 

class CPU {
public:
    // --- REGISTRADORES DO HARDWARE ---
    uint16_t PC;
    uint8_t  SP;
    uint8_t  A;
    uint8_t  X;
    uint8_t  Y;
    uint8_t  Status;

    // --- A MEMÓRIA RAM VIRTUAL ---
    uint8_t memoria[65536];

    // --- FUNÇÕES PÚBLICAS ---
    CPU();
    void Reset();
    void ExecutarCiclo();
    std::string DisassembleInstrucao(uint16_t& endereco);

private:
    // --- FUNÇÕES INTERNAS DE HARDWARE ---
    void AtualizarFlagsZeroENegativo(uint8_t registrador);

    // Barramento de Memória (O Pedágio)
    uint8_t LerMemoria(uint16_t endereco);
    void EscreverMemoria(uint16_t endereco, uint8_t valor);

    // Modos de Endereçamento
    uint16_t ModoImediato();
    uint16_t ModoAbsoluto();
    uint16_t ModoRelativo();
    uint16_t ModoAbsolutoX();

    // Instruções Isoladas
    void Instrucao_LDA(uint16_t endereco);
    void Instrucao_LDY(uint16_t endereco);
    void Instrucao_STA(uint16_t endereco);
    void Instrucao_BPL(uint16_t endereco);
};