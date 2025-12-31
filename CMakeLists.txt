cmake_minimum_required(VERSION 3.20)
project(glslang_wasm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch glslang from Khronos
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG        vulkan-sdk-1.4.321.0
    GIT_SHALLOW    TRUE
)

# Disable stuff we don't need for smaller build
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glslang)

add_executable(glslang_wasm main.cpp)

target_include_directories(glslang_wasm PRIVATE
    ${glslang_SOURCE_DIR}
)

target_link_libraries(glslang_wasm PRIVATE
    glslang
    SPIRV
    glslang-default-resource-limits
)

if(EMSCRIPTEN)
    set_target_properties(glslang_wasm PROPERTIES
        SUFFIX ".html"
    )
    target_link_options(glslang_wasm PRIVATE
        -sWASM=1
        -sTOTAL_MEMORY=134217728
        -sNO_EXIT_RUNTIME=1
        -sASSERTIONS=2
        -sSAFE_HEAP=0
        -sSTACK_OVERFLOW_CHECK=1
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
        -sASYNCIFY
        -sASYNCIFY_IMPORTS=[emscripten_sleep]
        -sASYNCIFY_STACK_SIZE=131072
    )
endif()
