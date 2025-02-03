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

  int y = cell->texture == TEX_WATER
              ? (cell->y + (1 - cell->fill_level)) * CELL_SIZE
              : cell->y * CELL_SIZE;
  int h = cell->texture == TEX_WATER ? cell->fill_level * CELL_SIZE : CELL_SIZE;
  SDL_RenderFillRect(
      renderer,
      &(SDL_Rect){.x = cell->x * CELL_SIZE, .y = y, .w = CELL_SIZE, .h = h});
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
  if (texture == TEX_WATER) {
    WORLD[y * CELL_COUNT_W + x].fill_level = 1.0;
  }
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
      WORLD[h * CELL_COUNT_W + w] =
          (cell){.x = w, .y = h, .texture = TEX_NONE, .fill_level = 0.0};
    }
  }
}

void fluid_simulate_step(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      cell *c = &WORLD[h * CELL_COUNT_W + w];
      if (c->texture != TEX_WATER)
        continue;

      int here_available = c->fill_level;
      if (here_available < 0)
        continue;

      int right_below = (h + 1) * CELL_COUNT_W + w;
      if (right_below >= CELL_COUNT_W * CELL_COUNT_H)
        continue;

      cell *c_below = &WORLD[right_below];
      int below_available = 1 - c_below->fill_level;

      if (here_available >= below_available) {
        int filling = here_available - below_available;
        c->fill_level -= filling;
        c_below->fill_level += filling;
        c_below->texture = TEX_WATER;
      } else {
        int filling = here_available;
        c->fill_level -= filling;
        c_below->fill_level += filling;
        c_below->texture = TEX_WATER;
      }
    }
  }
}

void fluid_render(SDL_Renderer *renderer) {
  fluid_render_world(renderer);
  fluid_render_grid(renderer);
  fluid_simulate_step(renderer);
}
