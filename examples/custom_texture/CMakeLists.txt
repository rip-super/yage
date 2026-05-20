add_executable(custom_texture main.cpp)

add_custom_command(TARGET custom_texture POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/awesomeface.png ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/awesomeface.png
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/container.jpg   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/container.jpg
)

target_link_libraries(custom_texture PRIVATE yage_extras)