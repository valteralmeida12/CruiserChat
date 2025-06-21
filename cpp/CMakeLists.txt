cmake_minimum_required(VERSION 3.16)
project(CruiserChat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread")

include_directories(
    ${CMAKE_SOURCE_DIR}/llama.cpp
    ${CMAKE_SOURCE_DIR}/llama.cpp/include
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/include
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src/ggml-cpu
    ${CMAKE_SOURCE_DIR}/llama.cpp/common
    ${CMAKE_SOURCE_DIR}/src/tokenizer
)

set(LLAMA_SOURCES
    ${CMAKE_SOURCE_DIR}/llama.cpp/src/llama.cpp
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src/ggml.c
    ${CMAKE_SOURCE_DIR}/llama.cpp/common/common.cpp
)

set(GGML_SRCS
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src/ggml.c
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src/ggml-alloc.c
    ${CMAKE_SOURCE_DIR}/llama.cpp/ggml/src/ggml-quants.c
)

add_subdirectory(llama.cpp)

# -------------------
# BEGIN: Build Info Setup

# 1. Set the build number manually or dynamically (example: 1)
set(LLAMA_BUILD_NUMBER 1)

# 2. Get git commit hash if possible
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE LLAMA_BUILD_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# 3. Compiler info
set(BUILD_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# 4. Build target info
set(BUILD_TARGET "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# 5. Configure build-info.cpp from build-info.cpp.in
configure_file(
  ${CMAKE_SOURCE_DIR}/llama.cpp/common/build-info.cpp.in
  ${CMAKE_BINARY_DIR}/build-info.cpp
  @ONLY
)

# -------------------
# Add executable with the generated build-info.cpp included

add_executable(CruiserChat
    src/main.cpp
    src/tokenizer/tokenizer.cpp
    ${GGML_SRCS}
    ${CMAKE_SOURCE_DIR}/llama.cpp/src/llama.cpp
    ${CMAKE_SOURCE_DIR}/llama.cpp/common/common.cpp
    ${CMAKE_SOURCE_DIR}/llama.cpp/common/log.cpp  
    ${CMAKE_BINARY_DIR}/build-info.cpp
)

target_link_libraries(CruiserChat PRIVATE llama)
