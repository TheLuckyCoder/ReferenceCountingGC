cmake_minimum_required (VERSION 3.15)
project ("GarbageCollector")

set(CMAKE_CXX_STANDARD 17)

add_executable (GarbageCollector main.cpp info.cpp info.h gc.cpp gc.h ref.h)
