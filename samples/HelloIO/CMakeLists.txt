# HelloIO Sample

add_executable(HelloIO main.cpp)

target_link_libraries(HelloIO PRIVATE ToyFrameV)

# Windows-specific: console app for debug output
if(WIN32)
    # Keep as console app for testing I/O output
endif()

# Emscripten/WebGL specific settings
if(EMSCRIPTEN)
    set_target_properties(HelloIO PROPERTIES
        SUFFIX ".html"
    )
    
    # Generate shell file from template with sample name
    set(SAMPLE_NAME "Hello IO")
    configure_file(
        ${CMAKE_SOURCE_DIR}/web/template.html
        ${CMAKE_CURRENT_BINARY_DIR}/shell.html
        @ONLY
    )
    
    target_link_options(HelloIO PRIVATE
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=1
        -sASYNCIFY
        --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html
    )
endif()
