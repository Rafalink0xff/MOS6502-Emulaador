cmake_minimum_required(VERSION 3.20)
project(EmuladorNES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- CONFIGURAÇÃO DA SDL2 ---
# Verifica se a pasta existe, senão avisa o erro de forma clara
set(SDL2_PATH "C:/msys64/mingw64")

if(NOT EXISTS "${SDL2_PATH}/include/SDL2")
    message(FATAL_ERROR "Nao encontrei a SDL2 em ${SDL2_PATH}. Verifique o caminho!")
endif()

include_directories("${SDL2_PATH}/include")
link_directories("${SDL2_PATH}/lib")

# Lista de arquivos
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

# Linkagem
target_link_libraries(EmuladorNES SDL2 SDL2main)

if(WIN32)
    set_target_properties(EmuladorNES PROPERTIES LINK_FLAGS "-mconsole -Wl,--subsystem,console")
endif()

target_include_directories(EmuladorNES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})