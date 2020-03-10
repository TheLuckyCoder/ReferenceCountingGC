cmake_minimum_required (VERSION 3.15)
project ("GarbageCollector")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable (GarbageCollector main.cpp info.cpp info.h gc.cpp gc.h ref.h page.h)
