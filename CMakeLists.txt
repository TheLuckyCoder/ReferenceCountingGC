cmake_minimum_required (VERSION 3.15)
project ("GarbageCollector")

set(CMAKE_CXX_STANDARD 17)

add_executable (GarbageCollector main.cpp Info.cpp Info.h GC.cpp GC.h)
