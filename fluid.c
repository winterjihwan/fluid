#include "fluid.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include <stdio.h>
#include <stdlib.h>

static cell WORLD[CELL_COUNT_H * CELL_COUNT_W] = {0};

static void fluid_color_set(SDL_Renderer *renderer, const color *color) {
  SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, 255);
}

static void fluid_render_grid(SDL_Renderer *renderer) {
  fluid_color_set(renderer, &COLOR_GRAY);
  for (size_t i = CELL_SIZE; i <= WINDOW_H; i += CELL_SIZE)
    SDL_RenderDrawLine(renderer, 0, i, WINDOW_W, i + 1);
  for (size_t i = CELL_SIZE; i <= WINDOW_W; i += CELL_SIZE)
    SDL_RenderDrawLine(renderer, i, 0, i + 1, WINDOW_H);
}

static void fluid_render_cell(SDL_Renderer *renderer, cell *cell) {
  fluid_color_set(renderer, &cell->texture);
  SDL_RenderFillRect(renderer, &(SDL_Rect){.x = cell->x * CELL_SIZE,
                                           .y = cell->y * CELL_SIZE,
                                           .w = CELL_SIZE,
                                           .h = CELL_SIZE});
}

static void fluid_render_world(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      cell *c = &WORLD[h * CELL_COUNT_W + w];
      if (!c->none)
        fluid_render_cell(renderer, c);
    }
  }
}

static void fluid_cell_update(SDL_Renderer *renderer, int x, int y,
                              color texture, int erase) {
  x = x / CELL_SIZE;
  y = y / CELL_SIZE;
  WORLD[y * CELL_COUNT_W + x].texture = texture;
  WORLD[y * CELL_COUNT_W + x].none = erase;
}

void fluid_event_handle(SDL_Renderer *renderer, SDL_Event *e) {
  static int d, erase = 0;
  static int type = 1;
  if (e->type == SDL_MOUSEBUTTONDOWN)
    d = 1;
  if (e->type == SDL_MOUSEMOTION && d) {
    color c;
    switch (type) {
    case 1:
      c = TEX_BUCKET;
      erase = 0;
      break;
    case 2:
      c = TEX_WATER;
      erase = 0;
      break;
    case 3:
      c = TEX_NONE;
      erase = 1;
      break;
    default:
      fprintf(stdout, "Unknown Texture");
      exit(1);
    }
    fluid_cell_update(renderer, e->motion.x, e->motion.y, c, erase);
  }
  if (e->type == SDL_MOUSEBUTTONUP)
    d = 0;
  if (e->type == SDL_KEYDOWN) {
    if (e->key.keysym.scancode == SDL_SCANCODE_1) {
      type = 1;
    } else if (e->key.keysym.scancode == SDL_SCANCODE_2) {
      type = 2;
    } else if (e->key.keysym.scancode == SDL_SCANCODE_3) {
      type = 3;
    }
  }
}

void fluid_initialize_static(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      WORLD[h * CELL_COUNT_W + w] =
          (cell){.x = w, .y = h, .texture = TEX_NONE, .none = 1};
    }
  }
}

void fluid_render(SDL_Renderer *renderer) {
  fluid_render_world(renderer);
  fluid_render_grid(renderer);
}
