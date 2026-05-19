cmake_minimum_required(VERSION 3.20)
project(yage VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)

add_library(glad STATIC deps/glad/src/glad.c)
target_include_directories(glad PUBLIC deps/glad/include)

add_library(miniaudio STATIC deps/miniaudio/src/miniaudio.c)
target_include_directories(miniaudio
    PUBLIC deps/miniaudio/include
    PRIVATE deps/miniaudio/include
)

add_subdirectory(deps/glm/glm)

add_subdirectory(yage)
add_subdirectory(examples)