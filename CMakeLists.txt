cmake_minimum_required(VERSION 3.10)
project(CruiserChat)

set(CMAKE_CXX_STANDARD 17)
set(LLAMA_BUILD_COMMON On)

# GPU compilation flags
option(CUDA "Enable CUDA support" OFF)
option(METAL "Enable Metal support" OFF)
option(ROCM "Enable AMD ROCm/HIP support" OFF) 

# Map CUDA to GGML_CUDA for llama.cpp compatibility
if(CUDA)
    set(GGML_CUDA ON)
endif()

# Map METAL to GGML_METAL for llama.cpp compatibility
if(METAL)
    set(GGML_METAL ON)
endif()

# Map ROCM to GGML_ROCM for llama.cpp compatibility
if(ROCM)
    set(GGML_ROCM ON)
endif()

# Manual readline detection
find_path(READLINE_INCLUDE_DIR readline/readline.h
    PATHS /usr/include /usr/local/include
)

find_library(READLINE_LIBRARY NAMES readline
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

find_library(NCURSES_LIBRARY NAMES ncurses
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)


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
    ${READLINE_INCLUDE_DIR}
)

# Link readline and ncurses
target_link_libraries(CruiserChat PRIVATE ${READLINE_LIBRARY})
if(NCURSES_LIBRARY)
    target_link_libraries(CruiserChat PRIVATE ${NCURSES_LIBRARY})
endif()

find_package(Threads REQUIRED)
target_link_libraries(CruiserChat PRIVATE Threads::Threads)