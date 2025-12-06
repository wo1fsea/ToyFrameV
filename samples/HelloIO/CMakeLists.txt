# HelloIO Sample

add_executable(HelloIO main.cpp)

target_link_libraries(HelloIO PRIVATE ToyFrameV)

# Windows-specific: console app for debug output
if(WIN32)
    # Keep as console app for testing I/O output
endif()
