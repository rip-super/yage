add_executable(custom_shader main.cpp)

add_custom_command(TARGET custom_shader POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/custom.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/custom.vert
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/custom.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/custom.frag
)

target_compile_definitions(custom_shader PRIVATE SHADER_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
target_link_libraries(custom_shader PRIVATE yage_extras)