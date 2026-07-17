#pragma once
#include <cstdint>
#include <string>
#include "bus.h"

class CPU {
private:
    Bus* bus; // A CPU só aponta para o Bus agora

public:
    CPU(Bus* b); // Construtor atualizado

    // --- REGISTRADORES DO HARDWARE ---
    uint16_t PC;
    uint8_t  SP;
    uint8_t  A;
    uint8_t  X;
    uint8_t  Y;
    uint8_t  Status;

    // --- FUNÇÕES PÚBLICAS ---
    void Reset();
    void NMI();
    bool vblank_ativo;
    void GatilhoVBlank();
    int ExecutarCiclo();
    std::string DisassembleInstrucao(uint16_t& endereco);

private:
    // --- FUNÇÕES INTERNAS DE HARDWARE ---
    void AtualizarFlagsZeroENegativo(uint8_t registrador);
    void Push(uint8_t valor);
    uint8_t Pop();

    // --- BARRAMENTO DE MEMÓRIA (O Pedágio) ---
    uint8_t LerMemoria(uint16_t endereco);
    void EscreverMemoria(uint16_t endereco, uint8_t valor);

    // --- MODOS DE ENDEREÇAMENTO ---
    uint16_t ModoImediato();
    uint16_t ModoAbsoluto();
    uint16_t ModoAbsolutoX();
    uint16_t ModoAbsolutoY();
    uint16_t ModoRelativo();
    uint16_t ModoZeroPage();
    uint16_t ModoIndiretoY();
    uint16_t ModoIndireto();
    uint16_t ModoZeroPageX();

    // --- INSTRUÇÕES DA CPU ---
    void Instrucao_LDA(uint16_t endereco);
    void Instrucao_LDX(uint16_t endereco);
    void Instrucao_LDY(uint16_t endereco);
    void Instrucao_STA(uint16_t endereco);
    void Instrucao_STX(uint16_t endereco);
    void Instrucao_STY(uint16_t endereco);
    void Instrucao_JSR(uint16_t endereco);
    void Instrucao_RTS();
    void Instrucao_RTI();
    void Instrucao_JMP(uint16_t endereco);
    void Instrucao_PHA();
    void Instrucao_PLA();
    void Instrucao_SEC();
    void Instrucao_CLC();
    void Instrucao_ROR_A();

    void Instrucao_DEX();
    void Instrucao_DEY();
    void Instrucao_CMP(uint16_t endereco);
    void Instrucao_CPX(uint16_t endereco);
    void Instrucao_CPY(uint16_t endereco);
    void Instrucao_BIT(uint16_t endereco);
    void Instrucao_LSR_Acumulador();
    void Instrucao_ROL_Acumulador();
    void Instrucao_ROR(uint16_t endereco);
    void Instrucao_ASL_Acumulador();
    void Instrucao_ASL(uint16_t endereco);
    void Instrucao_INY(uint16_t endereco);
    void Instrucao_INC(uint16_t endereco);
    void Instrucao_DEC(uint16_t endereco);
    void Instrucao_SBC(uint16_t endereco);
    void Instrucao_ADC(uint16_t endereco);
    void Instrucao_TAY();
    void Instrucao_TYA();
    void Instrucao_TAX();
    void Instrucao_TXA();

    void Instrucao_BPL(uint16_t endereco);
    void Instrucao_BCS(uint16_t endereco);
    void Instrucao_BCC(uint16_t endereco);
    void Instrucao_BNE(uint16_t endereco);
    void Instrucao_BEQ(uint16_t endereco);
    void Instrucao_BMI(uint16_t endereco);
    void Instrucao_ORA(uint16_t endereco);
    void Instrucao_AND(uint16_t endereco);
    void Instrucao_EOR(uint16_t endereco);
    void Instrucao_ROL(uint16_t endereco);
    void Instrucao_LSR(uint16_t endereco);
};