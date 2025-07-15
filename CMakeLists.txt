cmake_minimum_required(VERSION 3.16)
project(ScrabbleGame)

set(CMAKE_CXX_STANDARD 17)

# Add the custom SDL3 paths to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/SDL3-3.2.16/x86_64-w64-mingw32"
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_image/SDL3_image-3.2.4/x86_64-w64-mingw32" 
    "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_ttf/SDL3_ttf-3.2.2/x86_64-w64-mingw32"
)

# Find SDL3 packages
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED) 
find_package(SDL3_ttf REQUIRED)

# Collect all source files
set(SOURCES
    main.cpp
    src/Core/Game.cpp
    src/Core/Board.cpp
    src/Core/Tile.cpp
    src/Core/Player.cpp
    src/Core/Dictionary.cpp
    src/UI/GameRenderer.cpp
)

# Add executable
add_executable(scrabble_game ${SOURCES})

# Link libraries
target_link_libraries(scrabble_game 
    SDL3::SDL3-shared
    SDL3_image::SDL3_image-shared
    SDL3_ttf::SDL3_ttf-shared
)

# Copy DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET scrabble_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_image.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_ttf.dll"
        $<TARGET_FILE_DIR:scrabble_game>
    )
endif()
