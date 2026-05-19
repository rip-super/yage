file(GLOB_RECURSE YAGE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(yage STATIC ${YAGE_SOURCES})

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert" BATCH_VERT_SRC)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag" BATCH_FRAG_SRC)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/yage/shader_source.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/yage/shader_source.h"
)

target_include_directories(yage
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/deps/glm
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
            ${CMAKE_SOURCE_DIR}/deps/earcut
            ${CMAKE_SOURCE_DIR}/deps/stb_image
            ${CMAKE_SOURCE_DIR}/deps/miniaudio/include
)

target_link_libraries(yage PUBLIC glfw glad glm miniaudio)

if(WIN32)
    target_link_libraries(yage PUBLIC opengl32)
elseif(APPLE)
    find_library(OPENGL_LIB OpenGL REQUIRED)
    target_link_libraries(yage PUBLIC ${OPENGL_LIB})
else()
    target_link_libraries(yage PUBLIC GL)
endif()