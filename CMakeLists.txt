cmake_minimum_required(VERSION 3.20)
project(EmuladorNES CXX)

# Força o CLion a usar o padrão estável moderno do C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Informa ao CMake todos os nossos novos módulos separados
add_executable(EmuladorNES
        main.cpp
        cpu.cpp
        cpu_modos.cpp
        cpu_instrucoes.cpp
        cpu_debug.cpp
        cpu.h
)

# FORÇA O WINDOWS A USAR O MODO CONSOLE (Resolve o erro do WinMain)
if(WIN32)
    # Adicionamos -municode e garantimos que o entry point seja o main
    set_target_properties(EmuladorNES PROPERTIES LINK_FLAGS "-mconsole -Wl,--subsystem,console")
endif()

# Garante que o executável consiga achar arquivos na mesma pasta
target_include_directories(EmuladorNES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})