add_executable(audio main.cpp)

add_custom_command(TARGET audio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/music.mp3
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/music.mp3
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/hit.wav
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hit.wav
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/pickup.wav
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pickup.wav
)

target_link_libraries(audio PRIVATE yage)