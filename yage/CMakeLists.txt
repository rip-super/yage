find_package(Threads REQUIRED)

find_library(OPENGL_LIB OpenGL REQUIRED)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert" BATCH_VERT_SRC)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag" BATCH_FRAG_SRC)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/yage/shader_source.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/yage/shader_source.h"
)

add_library(yage_core STATIC
    src/window.cpp
    src/input.cpp
    src/renderer.cpp
    src/extras/shader.cpp
)
target_include_directories(yage_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/deps/glm
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
            ${CMAKE_SOURCE_DIR}/deps/earcut
            ${CMAKE_SOURCE_DIR}/deps/stb_image
)
target_link_libraries(yage_core PUBLIC glfw glad glm)

if(WIN32)
    target_link_libraries(yage_core PUBLIC opengl32)
elseif(APPLE)
    find_library(OPENGL_LIB OpenGL REQUIRED)
    target_link_libraries(yage_core PUBLIC ${OPENGL_LIB})
else()
    target_link_libraries(yage_core PUBLIC GL)
endif()

add_library(yage_extras STATIC
    src/extras/camera.cpp
    src/extras/scene_manager.cpp
    src/extras/texture.cpp
    src/extras/serializer.cpp
)
target_include_directories(yage_extras
    PUBLIC  ${CMAKE_SOURCE_DIR}/deps/nlohmann
    PRIVATE ${CMAKE_SOURCE_DIR}/deps/stb_image
)
target_link_libraries(yage_extras PUBLIC yage_core)

add_library(yage_audio STATIC src/extras/audio.cpp)
target_link_libraries(yage_audio PUBLIC yage_core miniaudio)

add_library(yage_network STATIC src/extras/network.cpp)
target_include_directories(yage_network
    PUBLIC  ${CMAKE_SOURCE_DIR}/deps/asio
            ${CMAKE_SOURCE_DIR}/deps/nlohmann
)
target_compile_definitions(yage_network PUBLIC ASIO_STANDALONE)
target_link_libraries(yage_network PUBLIC yage_core Threads::Threads)

add_library(yage INTERFACE)
target_link_libraries(yage INTERFACE yage_core yage_extras yage_audio yage_network)