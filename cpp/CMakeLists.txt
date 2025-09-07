cmake_minimum_required(VERSION 3.10)
project(CruiserChat)

set(CMAKE_CXX_STANDARD 17)
set(LLAMA_BUILD_COMMON On)

# Add llama.cpp as a subproject
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp")

# Build your app
add_executable(
    CruiserChat
    source/chatbot.cpp
    source/main.cpp
)

# Link against llama.cpp targets
target_link_libraries(
    CruiserChat
    PRIVATE
    common llama ggml
)

# Add your include directories
target_include_directories(
    CruiserChat
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
