#include "cpu.h"

void CPU::AtualizarFlagsZeroENegativo(uint8_t registrador) {
    if (registrador == 0x00) { Status |= 0x02; }
    else                     { Status &= ~0x02; }

    if (registrador & 0x80)  { Status |= 0x80; }
    else                     { Status &= ~0x80; }
}

void CPU::Instrucao_LDA(uint16_t endereco) {
    A = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_LDX(uint16_t endereco) {
    X = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(X);
}

void CPU::Instrucao_ROR_A() {
    uint8_t carry_antigo = (Status & 0x01); // Salva o Carry atual
    uint8_t bit0 = (A & 0x01);             // Salva o bit 0 (que será o novo Carry)

    // Faz a rotação
    A = (A >> 1);
    if (carry_antigo) A |= 0x80; // Põe o carry antigo no bit 7

    // Atualiza o Carry com o bit 0 antigo
    if (bit0) Status |= 0x01; else Status &= ~0x01;

    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_LDY(uint16_t endereco) {
    Y = LerMemoria(endereco);
    AtualizarFlagsZeroENegativo(Y);
}

void CPU::Instrucao_STA(uint16_t endereco) {
    EscreverMemoria(endereco, A);
}

void CPU::Instrucao_STX(uint16_t endereco) {
    EscreverMemoria(endereco, X);
}

void CPU::Instrucao_JMP(uint16_t endereco) {
    PC = endereco;
}

void CPU::Instrucao_BPL(uint16_t endereco) {
    if ((Status & 0x80) == 0) { PC = endereco; }
}

void CPU::Instrucao_BCS(uint16_t endereco) {
    if ((Status & 0x01)!= 0) { PC = endereco; }
}

void CPU::Instrucao_BNE(uint16_t endereco) {
    if ((Status & 0x02) == 0) { PC = endereco; }
}

void CPU::Instrucao_CMP(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = A - valor;

    if (A >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_CPX(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = X - valor;

    if (X >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_CPY(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t resultado = Y - valor;

    if (Y >= valor) { Status |= 0x01; }
    else            { Status &= ~0x01; }

    AtualizarFlagsZeroENegativo(resultado);
}

void CPU::Instrucao_PHA() {
    Push(A); // Salva o A na pilha mágica
}

void CPU::Instrucao_PLA() {
    A = Pop(); // Resgata o valor do topo da pilha e joga no A
    AtualizarFlagsZeroENegativo(A); // Informa o painel de Status sobre o novo valor
}

void CPU::Instrucao_SBC(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);

    // O 6502 transforma a subtração numa soma invertendo os bits do valor
    uint16_t valor_invertido = valor ^ 0xFF;
    uint8_t carry = Status & 0x01; // Pega o bit de Carry atual

    // Fazemos a soma: Acumulador + Valor Invertido + Carry
    uint16_t resultado = A + valor_invertido + carry;

    // 1. Atualiza o Carry (Se a soma passou de 255, a subtração deu certo)
    if (resultado > 255) { Status |= 0x01; }
    else                 { Status &= ~0x01; }

    // 2. Atualiza a Flag de Overflow (V) - Bit 6 do Status
    // A matemática maluca do 6502: se os sinais inverteram errado, liga o Overflow.
    if (~(A ^ valor_invertido) & (A ^ resultado) & 0x80) {
        Status |= 0x40; // Liga Overflow
    } else {
        Status &= ~0x40; // Desliga Overflow
    }

    // 3. Salva no Acumulador e atualiza Zero e Negativo
    A = (uint8_t)resultado;
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_BCC(uint16_t endereco) {
    // Se a Flag de Carry (Bit 0) estiver APAGADA (0), faz o pulo!
    if ((Status & 0x01) == 0) {
        PC = endereco;
    }
}

void CPU::Instrucao_DEC(uint16_t endereco) {
    // 1. Lê o valor atual da gaveta de memória
    uint8_t valor = LerMemoria(endereco);

    // 2. Subtrai 1 (Decrementa)
    valor--;

    // 3. Guarda o valor atualizado de volta na mesma gaveta
    EscreverMemoria(endereco, valor);

    // 4. E não podemos esquecer: as Flags reagem ao resultado!
    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_INC(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    valor++;
    EscreverMemoria(endereco, valor);
    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_BEQ(uint16_t endereco) {
    // Se a Flag Zero (Bit 1) estiver ACESA (1), fazemos o pulo!
    if ((Status & 0x02) != 0) {
        PC = endereco;
    }
}

void CPU::Instrucao_SEC() {
    Status |= 0x01; // Força o Bit 0 (Carry) a virar 1
}

void CPU::Instrucao_BIT(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);

    uint8_t resultado_and = A & valor;
    if (resultado_and == 0) { Status |= 0x02;  }
    else                    { Status &= ~0x02; }

    if (valor & 0x80) { Status |= 0x80;  }
    else              { Status &= ~0x80; }

    if (valor & 0x40) { Status |= 0x40;  }
    else              { Status &= ~0x40; }
}

void CPU::Instrucao_EOR(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);

    // Aplica o XOR (Ou Exclusivo) entre o Acumulador e o Valor
    A = A ^ valor;

    // Atualiza as Flags de Status
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_CLC() {
    Status &= ~0x01; // Força o Bit 0 (Carry) a apagar para 0
}

void CPU::Instrucao_ROR(uint16_t endereco) {
    // 1. Lê o valor da memória
    uint8_t valor = LerMemoria(endereco);

    // 2. Guarda o Carry antigo (se ele é 1, vai virar 128 (0x80) pra entrar no Bit 7)
    uint8_t carry_antigo = (Status & 0x01) ? 0x80 : 0x00;

    // 3. O Bit 0 atual vai ser o nosso novo Carry
    if (valor & 0x01) { Status |= 0x01;  } // Liga o Carry
    else              { Status &= ~0x01; } // Desliga o Carry

    // 4. Empurra todos os bits para a direita
    valor = valor >> 1;

    // 5. Preenche o Bit 7 com o Carry antigo
    valor = valor | carry_antigo;

    // 6. Escreve de volta na memória
    EscreverMemoria(endereco, valor);

    // 7. Atualiza o painel de Status
    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_ASL_Acumulador() {
    // 1. Salva o Bit 7 atual na Flag de Carry
    if (A & 0x80) { Status |= 0x01;  } // Se Bit 7 for 1, liga o Carry
    else          { Status &= ~0x01; } // Se for 0, desliga o Carry

    // 2. Empurra os bits para a Esquerda
    A = A << 1;

    // 3. Atualiza as Flags Zero e Negativo com o novo valor
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_TAY() {
    Y = A; // Copia o valor do Acumulador para o Y
    AtualizarFlagsZeroENegativo(Y); // O painel de Status precisa saber o que entrou no Y!
}

void CPU::Instrucao_TAX() {
    X = A;
    AtualizarFlagsZeroENegativo(X);
}

void CPU::Instrucao_TXA() {
    A = X;
    AtualizarFlagsZeroENegativo(A);
}

uint16_t CPU::ModoIndireto() {
    // 1. O PC aponta para a gaveta (2 bytes) que guarda o ponteiro
    uint8_t ponteiro_baixo = LerMemoria(PC);
    uint8_t ponteiro_alto = LerMemoria(PC + 1);
    PC += 2; // Avançamos o PC normalmente

    // 2. Juntamos o endereço do ponteiro
    uint16_t ponteiro = (ponteiro_alto << 8) | ponteiro_baixo;

    uint8_t byte_baixo = LerMemoria(ponteiro);
    uint8_t byte_alto;

    // 3. RECRIANDO O BUG DO HARDWARE DO 6502:
    // Se o ponteiro bater no limite de uma página (ex: 0x02FF)
    if (ponteiro_baixo == 0xFF) {
        // Ele "dá a volta" na mesma página em vez de ir pra próxima (lê 0x0200)
        byte_alto = LerMemoria(ponteiro & 0xFF00);
    } else {
        // Comportamento normal, vai pra próxima casinha (0x02FF + 1 = 0x0300)
        byte_alto = LerMemoria(ponteiro + 1);
    }

    // 4. Retorna o endereço real escondido lá dentro
    return (byte_alto << 8) | byte_baixo;
}

void CPU::Instrucao_ADC(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t carry = Status & 0x01;

    // A soma oficial: Acumulador + Valor + Carry
    uint16_t resultado = A + valor + carry;

    // 1. Atualiza o Carry (se passar de 255, a flag liga)
    if (resultado > 255) { Status |= 0x01; }
    else                 { Status &= ~0x01; }

    // 2. Atualiza o Overflow (V)
    // O sinal inverte se somarmos dois números iguais e o resultado for diferente
    if (!((A ^ valor) & 0x80) && ((A ^ resultado) & 0x80)) {
        Status |= 0x40;
    } else {
        Status &= ~0x40;
    }

    // 3. Salva e atualiza Z e N
    A = (uint8_t)resultado;
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_STY(uint16_t endereco) {
    EscreverMemoria(endereco, Y); // Pega o Y e guarda na gaveta!
}

void CPU::Instrucao_RTI() {
    // 1. Resgata o painel de Status inteiro que estava salvo na pilha
    Status = Pop();
    Status |= 0x20; // O Bit 5 no NES não existe fisicamente, mas a CPU lê sempre como 1

    // 2. Resgata o endereço de onde paramos (Primeiro o baixo, depois o alto)
    uint8_t byte_baixo = Pop();
    uint8_t byte_alto = Pop();

    // 3. Monta o endereço original e joga no Program Counter
    PC = (byte_alto << 8) | byte_baixo;
}

void CPU::Instrucao_TYA() {
    A = Y; // Copia Y para A
    AtualizarFlagsZeroENegativo(A); // Atualiza o status
}

void CPU::Instrucao_BMI(uint16_t endereco) {
    // Se a Flag de Negativo (Bit 7 - 0x80) estiver ACESA (1), fazemos o pulo!
    if ((Status & 0x80) != 0) {
        PC = endereco;
    }
}

void CPU::Instrucao_ROL(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);
    uint8_t carry_antigo = Status & 0x01;
    uint8_t bit7 = (valor & 0x80) >> 7;

    valor = (valor << 1) | carry_antigo;

    // Atualiza Carry
    if (bit7) Status |= 0x01; else Status &= ~0x01;

    EscreverMemoria(endereco, valor);
    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_LSR(uint16_t endereco) {
    uint8_t valor = LerMemoria(endereco);

    // O bit 0 vai para o Carry
    if (valor & 0x01) Status |= 0x01; // Carry = 1
    else Status &= ~0x01;            // Carry = 0

    valor >>= 1; // Shift para a direita

    EscreverMemoria(endereco, valor);
    AtualizarFlagsZeroENegativo(valor); // Flag Zero e Negativo
}

void CPU::Instrucao_LSR_Acumulador() {
    // 1. Salva o bit 0 na Flag de Carry (Status Bit 0)
    if (A & 0x01) { Status |= 0x01;  } // Liga o Carry se o bit for 1
    else          { Status &= ~0x01; } // Desliga o Carry se for 0

    // 2. Empurra os bits para a direita (o bit 7 automaticamente vira 0 em C++)
    A = A >> 1;

    // 3. O Zero Page Flags (Z e N) reagem ao resultado novo
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_ROL_Acumulador() {
    // 1. Salva o Carry Antigo para colocarmos no Bit 0 depois
    uint8_t carry_antigo = (Status & 0x01) ? 1 : 0;

    // 2. O Bit 7 atual vai ser a nossa nova Flag de Carry (C)
    if (A & 0x80) { Status |= 0x01;  } // Se Bit 7 for 1, liga a flag
    else          { Status &= ~0x01; } // Se for 0, desliga

    // 3. Empurra todos os bits para a esquerda
    A = A << 1;

    // 4. Coloca o Carry Antigo no Bit 0 vazio
    A = A | carry_antigo;

    // 5. Atualiza o painel de Status com as flags Z e N
    AtualizarFlagsZeroENegativo(A);
}

void CPU::Instrucao_AND(uint16_t endereco) {
    A = A & LerMemoria(endereco);

    if (A == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (A & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_ORA(uint16_t endereco) {
    A = A | LerMemoria(endereco);

    if (A == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (A & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_DEX() {
    X = X - 1;
    AtualizarFlagsZeroENegativo(X);
}

void CPU::Instrucao_DEY() {
    Y = Y - 1;
    AtualizarFlagsZeroENegativo(Y);
}

void CPU::Instrucao_INY(uint16_t endereco) {
    Y++;
    if (Y == 0) { Status |= 0x02;  }
    else        { Status &= ~0x02; }

    if (Y & 0x80) { Status |= 0x80;  }
    else          { Status &= ~0x80; }
}

void CPU::Instrucao_JSR(uint16_t endereco) {
    uint16_t endereco_retorno = PC - 1;
    Push((endereco_retorno >> 8) & 0xFF);
    Push(endereco_retorno & 0xFF);
    PC = endereco;
}

void CPU::Instrucao_RTS() {
    uint8_t byte_baixo = Pop();
    uint8_t byte_alto = Pop();
    uint16_t endereco_retorno = (byte_alto << 8) | byte_baixo;
    PC = endereco_retorno + 1;
}

void CPU::Instrucao_ASL(uint16_t endereco) {
    // 1. Lê o valor da memória
    uint8_t valor = LerMemoria(endereco);

    // 2. O Bit 7 (o mais à esquerda) vai cair fora e precisa ir para o Carry
    if (valor & 0x80) { Status |= 0x01;  } // Liga o Carry
    else              { Status &= ~0x01; } // Desliga o Carry

    // 3. Empurra todos os bits para a Esquerda
    valor = valor << 1;

    // 4. Salva de volta na memória e atualiza as flags Z e N
    EscreverMemoria(endereco, valor);
    AtualizarFlagsZeroENegativo(valor);
}

void CPU::Instrucao_PHP() {
    // Salva o painel de status com as flags B (Bit 4) e Unused (Bit 5) acesas
    Push(Status | 0x30);
}

void CPU::Instrucao_PLP() {
    // Restaura o painel de status, ignorando a flag B e forçando o Bit 5 aceso
    Status = (Pop() & 0xEF) | 0x20;
}

void CPU::Instrucao_BVC(uint16_t endereco) {
    // Pula se a flag de Overflow (Bit 6) estiver DESLIGADA
    if ((Status & 0x40) == 0) PC = endereco;
}

void CPU::Instrucao_BVS(uint16_t endereco) {
    // Pula se a flag de Overflow (Bit 6) estiver LIGADA
    if ((Status & 0x40) != 0) PC = endereco;
}