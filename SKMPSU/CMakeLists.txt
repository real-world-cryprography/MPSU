cmake_minimum_required(VERSION 3.15)
project(SKMPSU)

include_directories(common)
include_directories(offlineGen)
include_directories(shuffle)
include_directories(mpsu)

file(GLOB_RECURSE SRCS
    common/*.cpp
    offlineGen/*.cpp
    shuffle/*.cpp
    mpsu/*.cpp
)

add_executable(main ${SRCS})

find_package(volePSI REQUIRED HINTS "../libvolepsi")

set(CMAKE_BUILD_TYPE Release)

target_compile_options(main PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(main visa::volePSI)



