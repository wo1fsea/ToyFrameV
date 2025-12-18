# HelloTexture Sample
add_executable(HelloTexture main.cpp)
target_link_libraries(HelloTexture PRIVATE ToyFrameV)

# Emscripten-specific settings
if(EMSCRIPTEN)
    set_target_properties(HelloTexture PROPERTIES
        SUFFIX ".html"
    )
    target_link_options(HelloTexture PRIVATE
        --shell-file ${CMAKE_SOURCE_DIR}/web/template.html
        -sALLOW_MEMORY_GROWTH=1
        -sEXPORTED_FUNCTIONS=['_main']
        -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    )
endif()
