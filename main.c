#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  if (SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL2 init fail");
    return EXIT_FAILURE;
  }

  SDL_Window *window =
      SDL_CreateWindow("Simple SDL2", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1600, 900, SDL_WINDOW_SHOWN);

  if (!window) {
    fprintf(stderr, "SDL2 create window fail");
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    fprintf(stderr, "SDL2 create renderer fail");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Event e;
  int should_end = 0;
  while (!should_end) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        should_end = 1;
      }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}
