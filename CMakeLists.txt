cmake_minimum_required(VERSION 3.15)
project("GarbageCollector")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wunused -Woverloaded-virtual -Wpedantic -Wnull-dereference")

add_executable(
        GarbageCollector
        src/main.cpp

        src/gc/gc.cpp
)
