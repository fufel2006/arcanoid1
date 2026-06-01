#include "GameManager.h"
#include <SDL.h>

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    window = SDL_CreateWindow("Arkanoid", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    GameManager game;
    game.init();

    bool spacePressed = false;
    bool rPressed = false;

    Uint32 frameStart;
    int frameTime;

    while (game.isRunning()) {
        frameStart = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game.setRunning(false);
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);

        bool currentSpace = keys[SDL_SCANCODE_SPACE];
        bool currentR = keys[SDL_SCANCODE_R];

        game.handleInput(keys, currentSpace && !spacePressed, currentR && !rPressed);

        spacePressed = currentSpace;
        rPressed = currentR;

        game.update();
        game.render(renderer);

        frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}