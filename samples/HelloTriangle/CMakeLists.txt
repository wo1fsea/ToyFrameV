# HelloTriangle Sample
# 
# Note: Only links to ToyFrameV - no direct LLGL dependency needed!

add_executable(HelloTriangle main.cpp)

target_link_libraries(HelloTriangle PRIVATE ToyFrameV)

# Emscripten/WebGL specific settings
if(EMSCRIPTEN)
    set_target_properties(HelloTriangle PROPERTIES
        SUFFIX ".html"
    )
    
    # Generate shell file from template with sample name
    set(SAMPLE_NAME "Hello Triangle")
    configure_file(
        ${CMAKE_SOURCE_DIR}/web/template.html
        ${CMAKE_CURRENT_BINARY_DIR}/shell.html
        @ONLY
    )
    
    target_link_options(HelloTriangle PRIVATE
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=1
        -sASYNCIFY
        --shell-file ${CMAKE_CURRENT_BINARY_DIR}/shell.html
    )
endif()
