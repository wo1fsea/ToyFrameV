cmake_minimum_required(VERSION 3.20)

add_executable(HelloThreadLog
    main.cpp
)

target_link_libraries(HelloThreadLog PRIVATE ToyFrameV)

set_target_properties(HelloThreadLog PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
