cmake_minimum_required(VERSION 3.5)
set(PROJECT_NAME "SimpleGame")

project("SimpleGame")


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Bin)

find_package(Gorgon REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

linkgorgon(${PROJECT_NAME})

