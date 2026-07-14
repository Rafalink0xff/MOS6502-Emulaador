cmake_minimum_required(VERSION 3.20)
project(EmuladorNES CXX)

# Força o CLion a usar o padrão estável moderno do C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Informa ao CMake quais arquivos fonte compõem o nosso executável
add_executable(EmuladorNES
        main.cpp
        cpu.cpp
        cpu.h
)

# Garante que o executável consiga achar arquivos na mesma pasta
target_include_directories(EmuladorNES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})