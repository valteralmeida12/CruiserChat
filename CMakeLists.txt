cmake_minimum_required(VERSION 3.10)
project(CruiserChat)

set(CMAKE_CXX_STANDARD 17)
set(LLAMA_BUILD_COMMON On)

# GPU compilation flags
set(CUDA ON CACHE BOOL "Enable CUDA support" FORCE)

# Map CUDA to GGML_CUDA for llama.cpp compatibility
if(CUDA)
    set(GGML_CUDA ON CACHE BOOL "Enable CUDA support" FORCE)
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp")

add_executable(
    CruiserChat
    source/chatbot.cpp
    source/main.cpp
)

target_link_libraries(
    CruiserChat
    PRIVATE
    common llama ggml
)

target_include_directories(
    CruiserChat
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(CruiserChat PRIVATE Threads::Threads)