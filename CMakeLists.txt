cmake_minimum_required(VERSION 3.15)
project("GarbageCollector")

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_compile_options(/permissive- /W4 /WX)
else()
    add_compile_options(-pthread -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wunused -Woverloaded-virtual -pedantic -Wnull-dereference -Werror)
endif()

add_executable(
        GarbageCollector
        src/main.cpp

        src/gc/gc.cpp
)
