cmake_minimum_required(VERSION 3.20)

add_executable(HelloTimer
    main.cpp
)

target_link_libraries(HelloTimer PRIVATE ToyFrameV)

set_target_properties(HelloTimer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Emscripten/WebGL specific settings
if(EMSCRIPTEN)
    set_target_properties(HelloTimer PROPERTIES
        SUFFIX ".html"
    )

    set(SAMPLE_NAME "Hello Timer")
    configure_file(
        ${CMAKE_SOURCE_DIR}/web/template.html
        ${CMAKE_CURRENT_BINARY_DIR}/shell.html
        @ONLY
    )

    target_link_options(HelloTimer PRIVATE
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=1
        -sASYNCIFY
        --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html
    )
endif()
