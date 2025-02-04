cmake_minimum_required(VERSION 3.15)
project(PKMPSU)


IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(OPENSSL_LIBRARIES /usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libssl.a)
    SET(OPENSSL_INCLUDE_DIR /usr/include/openssl)
    SET(OPENSSL_DIR /usr/lib/x86_64-linux-gnu)
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    SET(OPENSSL_LIBRARIES /usr/local/lib/libcrypto.a /usr/local/lib/libssl.a)
    SET(OPENSSL_INCLUDE_DIR /usr/local/include)
    SET(OPENSSL_DIR /usr/local/lib)
ENDIF()

FIND_PACKAGE(OpenMP REQUIRED)

# FIND_PACKAGE(absl REQUIRED)

add_definitions(-w)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fpic -Wno-deprecated-declarations -Wno-unused-result -Xpreprocessor -O0")

SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

INCLUDE_DIRECTORIES(
    ${OPENSSL_INCLUDE_DIR}
    "/usr/local/include"
)

LINK_DIRECTORIES(
    /usr/local/lib
)

find_package(volePSI REQUIRED HINTS "../libvolepsi")

file(GLOB_RECURSE SRCS
    common/*.cpp
    offlineGen/*.cpp
    ECPoint/*.cpp    
    mpsu/*.cpp
)

set(CMAKE_BUILD_TYPE Release)


add_executable(main ${SRCS})
target_compile_options(main PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(main visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)












