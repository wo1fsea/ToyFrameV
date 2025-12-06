# HelloApp Sample

add_executable(HelloApp main.cpp)

target_link_libraries(HelloApp PRIVATE ToyFrameV)

# Windows-specific: use console for debug output
if(WIN32)
    # Keep as console app for Stage 1 testing
    # Will switch to WIN32_EXECUTABLE in Stage 2 when window is created
endif()
