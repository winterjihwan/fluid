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

static color fluid_tex_color(const texture t) {
  switch (t) {
  case TEX_NONE:
    return COLOR_BLACK;
  case TEX_BUCKET:
    return COLOR_WHITE;
  case TEX_WATER:
    return COLOR_LBLUE;
  }
}

static void fluid_render_cell(SDL_Renderer *renderer, cell *cell) {
  if (cell->texture == TEX_NONE)
    return;

  color color = fluid_tex_color(cell->texture);
  fluid_color_set(renderer, &color);
  SDL_RenderFillRect(renderer, &(SDL_Rect){.x = cell->x * CELL_SIZE,
                                           .y = cell->y * CELL_SIZE,
                                           .w = CELL_SIZE,
                                           .h = CELL_SIZE});
}

static void fluid_render_world(SDL_Renderer *renderer) {
  for (size_t p = 0; p < CELL_COUNT_H * CELL_COUNT_W; p++)
    fluid_render_cell(renderer, &WORLD[p]);
}

static void fluid_cell_update(SDL_Renderer *renderer, int x, int y,
                              texture texture) {
  x = x / CELL_SIZE;
  y = y / CELL_SIZE;
  WORLD[y * CELL_COUNT_W + x].texture = texture;
}

void fluid_event_handle(SDL_Renderer *renderer, SDL_Event *e) {
  static int d;
  static texture texture = TEX_NONE;
  if (e->type == SDL_MOUSEBUTTONDOWN)
    d = 1;
  if (e->type == SDL_MOUSEMOTION && d) {
    color c;
    fluid_cell_update(renderer, e->motion.x, e->motion.y, texture);
  }
  if (e->type == SDL_MOUSEBUTTONUP)
    d = 0;
  if (e->type == SDL_KEYDOWN) {
    if (e->key.keysym.scancode == SDL_SCANCODE_1) {
      texture = TEX_BUCKET;
    } else if (e->key.keysym.scancode == SDL_SCANCODE_2) {
      texture = TEX_WATER;
    } else if (e->key.keysym.scancode == SDL_SCANCODE_3) {
      texture = TEX_NONE;
    }
  }
}

void fluid_initialize_static(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      WORLD[h * CELL_COUNT_W + w] = (cell){.x = w, .y = h, .texture = TEX_NONE};
    }
  }
}

void fluid_simulate_step(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      cell *c = &WORLD[h * CELL_COUNT_W + w];
    }
  }
}

void fluid_render(SDL_Renderer *renderer) {
  fluid_render_world(renderer);
  fluid_render_grid(renderer);
}
