#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

// Link SDL3 libraries on Windows
#ifdef _WIN32
    #pragma comment(lib, "SDL3.lib")
    #pragma comment(lib, "SDL3main.lib")
    #undef main
#endif

int main(int argc, char* argv[])
{
    // SDL3 uses SDL_Init() with specific subsystems
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // SDL3 window creation syntax is different
    SDL_Window *window = SDL_CreateWindow("Hello SDL3", 800, 600, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                {
                    isRunning = false;
                }
                break;
            }
        }

        // SDL3 uses different color setting - set before clear
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}