cmake_minimum_required(VERSION 3.20)
project(EmuladorNES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- CONFIGURAÇÃO MANUAL DA SDL2 ---
# Ajuste este caminho se o seu MSYS2 não estiver em C:/msys64
set(SDL2_ROOT "C:/msys64/mingw64")

include_directories(${SDL2_ROOT}/include)
link_directories(${SDL2_ROOT}/lib)

add_executable(EmuladorNES
        main.cpp
        cpu.cpp
        cpu_modos.cpp
        cpu_instrucoes.cpp
        cpu_debug.cpp
        bus.cpp
        ppu.cpp
        ppu.h
)

# Linka a SDL2 e a biblioteca de threads que ela precisa
target_link_libraries(EmuladorNES SDL2 SDL2main)

if(WIN32)
    # Garante que abra o console + a janela gráfica
    set_target_properties(EmuladorNES PROPERTIES
            LINK_FLAGS "-mconsole -Wl,--subsystem,console"
    )

    target_include_directories(EmuladorNES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})