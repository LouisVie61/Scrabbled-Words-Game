#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include "src/Core/Game.hpp"

#ifdef _WIN32
    #pragma comment(lib, "SDL3.lib")
    #pragma comment(lib, "SDL3main.lib")
    #pragma comment(lib, "SDL3_image.lib")
    #pragma comment(lib, "SDL3_ttf.lib")
    #undef main
#endif

int main(int argc, char* argv[])
{
    // Initialize SDL subsystems early
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    
    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf initialization failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    Game game;
    
    if (!game.initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Option 1: Human vs Human 
    // if (!game.setupGame(GameMode::HUMAN_VS_HUMAN, "Alice", "Bob")) {
    //     std::cerr << "Failed to setup game!" << std::endl;
    //     TTF_Quit();
    //     SDL_Quit();
    //     return 1;
    // }

    std::cout << "Game initialized successfully!" << std::endl;
    std::cout << "Starting with menu screen..." << std::endl;
    
    
    /* Option 2: Human vs AI (testing)
    if (!game.setupGame(GameMode::HUMAN_VS_AI, "Player", "Computer")) {
        std::cerr << "Failed to setup game!" << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    */
    
    /* Option 3: AI vs AI (for testing)
    if (!game.setupGame(GameMode::AI_VS_AI, "AI Easy", "AI Hard")) {
        std::cerr << "Failed to setup game!" << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    */
    
    // Run the game loop
    game.run();
    TTF_Quit();
    SDL_Quit();
    
    return 0;
    }