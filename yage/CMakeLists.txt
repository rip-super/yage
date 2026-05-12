file(GLOB_RECURSE YAGE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(yage STATIC ${YAGE_SOURCES})

target_include_directories(yage
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(yage
    PUBLIC glfw
           glad
           glm
)

if(WIN32)
    target_link_libraries(yage PUBLIC opengl32)
elseif(APPLE)
    find_library(OPENGL_LIB OpenGL REQUIRED)
    target_link_libraries(yage PUBLIC ${OPENGL_LIB})
else()
    target_link_libraries(yage PUBLIC GL)
endif()