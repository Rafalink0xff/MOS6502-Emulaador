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
    void Push(uint8_t valor);

    // --- BARRAMENTO DE MEMÓRIA (O Pedágio) ---
    uint8_t LerMemoria(uint16_t endereco);
    void EscreverMemoria(uint16_t endereco, uint8_t valor);

    // --- MODOS DE ENDEREÇAMENTO ---
    uint16_t ModoImediato();
    uint16_t ModoAbsoluto();
    uint16_t ModoAbsolutoX();
    uint16_t ModoRelativo();
    uint16_t ModoZeroPage();

    // --- INSTRUÇÕES DA CPU ---
    // Load / Store (Memória)
    void Instrucao_LDA(uint16_t endereco);
    void Instrucao_LDY(uint16_t endereco);
    void Instrucao_STA(uint16_t endereco);
    void Instrucao_JSR(uint16_t endereco);
    void Instrucao_STX(uint16_t endereco);

    // Matemática e Registradores
    void Instrucao_DEX(); // Corrigido: Sem endereço!
    void Instrucao_CMP(uint16_t endereco);
    void Instrucao_CPX(uint16_t endereco);

    // Desvios Condicionais (Branches)
    void Instrucao_BPL(uint16_t endereco);
    void Instrucao_BCS(uint16_t endereco);
    void Instrucao_BNE(uint16_t endereco);
};