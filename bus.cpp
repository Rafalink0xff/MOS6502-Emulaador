#include "bus.h"

Bus::Bus() {
    controle_estado = 0;
    controle_shift = 0;
    for (int i = 0; i < 2048; i++) cpuRam[i] = 0;
    for (int i = 0; i < 32768; i++) cartucho[i] = 0;
}

uint8_t Bus::Ler(uint16_t endereco) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        // A RAM original do NES só tem 2KB, mas é espelhada até 0x1FFF
        return cpuRam[endereco & 0x07FF];
    }
    else if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        // Comunicação com a Placa de Vídeo (PPU)
        return ppu.LerRegistrador(endereco & 0x2007);
    }
    else if (endereco == 0x4016) {
        // --- LEITURA DO JOYPAD 1 ---
        // O NES lê um botão por vez. Pegamos o bit 7, depois empurramos a fila.
        uint8_t botao = (controle_shift & 0x80) > 0;
        controle_shift <<= 1;
        return botao;
    }
    else if (endereco >= 0x8000 && endereco <= 0xFFFF) {
        // Onde o código real do jogo (Super Mario Bros) fica armazenado
        return cartucho[endereco - 0x8000];
    }

    // Se o jogo tentar ler um lugar vazio ou não mapeado, retorna 0
    return 0;
}

void Bus::Escrever(uint16_t endereco, uint8_t valor) {
    if (endereco >= 0x0000 && endereco <= 0x1FFF) {
        cpuRam[endereco & 0x07FF] = valor;
    }
    else if (endereco >= 0x2000 && endereco <= 0x3FFF) {
        ppu.EscreverRegistrador(endereco & 0x2007, valor);
    }
    else if (endereco == 0x4014) {
        // --- OAM DMA (Transferência rápida de Sprites) ---
        uint16_t pagina_origem = valor << 8;
        for (int i = 0; i < 256; i++) {
            ppu.oam[i] = Ler(pagina_origem + i);
        }
    }
    else if (endereco == 0x4016) {
        // --- GATILHO DO JOYPAD 1 ---
        // O jogo escreve 1 e depois 0 para "bater a foto" do teclado.
        // Nós capturamos a foto toda vez que ele escreve qualquer coisa aqui.
        controle_shift = controle_estado;
    }
}