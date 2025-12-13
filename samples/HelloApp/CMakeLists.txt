# HelloApp Sample

add_executable(HelloApp main.cpp)

target_link_libraries(HelloApp PRIVATE ToyFrameV)

# Windows-specific: use console for debug output
if(WIN32)
    # Keep as console app for Stage 1 testing
    # Will switch to WIN32_EXECUTABLE in Stage 2 when window is created
endif()

# Emscripten/WebGL specific settings
if(EMSCRIPTEN)
    set_target_properties(HelloApp PROPERTIES
        SUFFIX ".html"
    )
    
    # Generate shell file from template with sample name
    set(SAMPLE_NAME "Hello App")
    configure_file(
        ${CMAKE_SOURCE_DIR}/web/template.html
        ${CMAKE_CURRENT_BINARY_DIR}/shell.html
        @ONLY
    )
    
    target_link_options(HelloApp PRIVATE
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=1
        -sASYNCIFY
        --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html
    )
endif()
