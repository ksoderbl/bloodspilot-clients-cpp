# src/CMakeLists.txt
add_subdirectory(common)
add_subdirectory(client)

if(BLOODSPILOT_BUILD_REPLAY)
    add_subdirectory(replay)
endif()
