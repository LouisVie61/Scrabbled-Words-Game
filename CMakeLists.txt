cmake_minimum_required(VERSION 3.16)
project(ScrabbleGame)

set(CMAKE_CXX_STANDARD 17)

# Find SDL3 - note SDL3 uses different package names
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)

# Add executable
add_executable(scrabble_game main.cpp)

# Link libraries - SDL3 has different target names
target_link_libraries(scrabble_game 
    SDL3::SDL3 
    SDL3::SDL3main
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
)

# Copy DLLs to output directory on Windows
if(WIN32)
    add_custom_command(TARGET scrabble_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_image.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_ttf.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL3_mixer.dll"
        $<TARGET_FILE_DIR:scrabble_game>)
endif()
