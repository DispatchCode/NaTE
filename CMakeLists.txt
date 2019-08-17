cmake_minimum_required(VERSION 3.14)
project(codegolf_8086 C)

set(CMAKE_C_STANDARD 11)

include_directories(src/include)

add_executable(codegolf_8086
                    src/ild.c
                    src/main.c
                    src/cpu.c
                    src/cpu_exec.c
                )
