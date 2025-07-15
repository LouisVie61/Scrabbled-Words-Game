#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "src/Core/Game.hpp"

// Link SDL3 libraries on Windows
#ifdef _WIN32
    #pragma comment(lib, "SDL3.lib")
    #pragma comment(lib, "SDL3main.lib")
    #undef main
#endif

int main(int argc, char* argv[])
{
    Game game;
    
    if (!game.initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    // Option 1: Human vs Human
    if (!game.setupGame(GameMode::HUMAN_VS_HUMAN, "Alice", "Bob")) {
        std::cerr << "Failed to setup game!" << std::endl;
        return 1;
    }
    
    /* Option 2: Human vs AI (testing)
    if (!game.setupGame(GameMode::HUMAN_VS_AI, "Player", "Computer")) {
        std::cerr << "Failed to setup game!" << std::endl;
        return 1;
    }
    */
    
    /* Option 3: AI vs AI (for testing)
    if (!game.setupGame(GameMode::AI_VS_AI, "AI Easy", "AI Hard")) {
        std::cerr << "Failed to setup game!" << std::endl;
        return 1;
    }
    */
    
    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Press SPACE to skip turn, ESC to quit" << std::endl;
    
    // Run the game loop
    game.run();
    
    return 0;
}