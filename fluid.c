#include "fluid.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include <stdio.h>
#include <stdlib.h>

static cell WORLD[CELL_COUNT_H * CELL_COUNT_W] = {0};
static cell SECOND_WORLD[CELL_COUNT_H * CELL_COUNT_W] = {0};

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
  case TEX_BUCKET:
    return COLOR_WHITE;
  case TEX_WATER:
    return COLOR_LBLUE;
  }
}

static void fluid_render_cell(SDL_Renderer *renderer, cell *cell) {
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

void fluid_initialize_static(SDL_Renderer *renderer) {
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      WORLD[h * CELL_COUNT_W + w] =
          (cell){.x = w, .y = h, .texture = TEX_WATER, .fill_level = 0.0};
      SECOND_WORLD[h * CELL_COUNT_W + w] =
          (cell){.x = w, .y = h, .texture = TEX_WATER, .fill_level = 0.0};
    }
  }
}

void fluid_simulate_step(SDL_Renderer *renderer) {
  WORLD_MOV(WORLD, SECOND_WORLD);
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      int here = h * CELL_COUNT_W + w;
      cell *c_here = &WORLD[here];
      if (c_here->texture != TEX_WATER)
        continue;

      float here_fill = c_here->fill_level;
      if (here_fill == 0.0f)
        continue;

      // Rule #1: Flow below
      int below = (h + 1) * CELL_COUNT_W + w;
      cell *c2_here = &SECOND_WORLD[here];
      cell *c_below = &WORLD[below];
      cell *c2_below = &SECOND_WORLD[below];

      if (below >= CELL_COUNT_W * CELL_COUNT_H) {
        continue;
      }
      if (c_below->texture == TEX_BUCKET) {
        continue;
      }

      float below_fill = c_below->fill_level;
      if (here_fill <= below_fill)
        continue;

      float to_fill = here_fill - below_fill;
      c2_here->fill_level -= to_fill;
      c2_below->fill_level += to_fill;

      // Rule #2: Flow left and right
      if (w <= 0 || w >= CELL_COUNT_W)
        continue;

      int left = h * CELL_COUNT_W + w - 1;
      int right = h * CELL_COUNT_W + w + 1;
      cell *c_left = &WORLD[left];
      cell *c_right = &WORLD[right];
      cell *c2_left = &SECOND_WORLD[left];
      cell *c2_right = &SECOND_WORLD[right];

      float left_fill = c_left->fill_level;
      float right_fill = c_right->fill_level;

      int can_lfill = c_left->texture != TEX_BUCKET && here_fill > left_fill;
      int can_rfill = c_right->texture != TEX_BUCKET && here_fill > right_fill;

      to_fill = 0;
      float to_lfill = 0;
      float to_rfill = 0;

      if (!can_lfill && !can_rfill)
        continue;

      int how_much;
      if (can_lfill && !can_rfill) {
        how_much = (here_fill - left_fill) / 2;
        to_fill += how_much;
        to_lfill += how_much;
      } else if (!can_lfill && can_rfill) {
        how_much = (here_fill - right_fill) / 2;
        to_fill += how_much;
        to_rfill += how_much;
      } else if (can_lfill && can_rfill) {
        how_much = (here_fill - left_fill) / 4;
        to_fill += how_much;
        to_lfill += how_much;

        how_much = (here_fill - right_fill) / 4;
        to_fill += how_much;
        to_rfill += how_much;
      }

      c2_here->fill_level -= to_fill;
      c2_left->fill_level += to_lfill;
      c2_right->fill_level += to_rfill;
    }
  }
  WORLD_MOV(SECOND_WORLD, WORLD);
}

void fluid_simulate_above_step(SDL_Renderer *renderer) {
  WORLD_MOV(WORLD, SECOND_WORLD);
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      int here = h * CELL_COUNT_W + w;
      cell *c = &WORLD[here];
      if (c->texture != TEX_WATER)
        continue;

      float here_fill = c->fill_level;
      if (here_fill == 0)
        continue;

      if (h <= 0) {
        fprintf(stdout, "Overflow water above screen\n");
        exit(1);
      }

      int above = (h - 1) * CELL_COUNT_W + w;
      cell *second_c = &SECOND_WORLD[here];
      cell *second_c_above = &SECOND_WORLD[above];

      // Rule #3: Flow above
      if (here_fill > 1) {
        float to_fill = here_fill - 1.0;
        second_c->fill_level -= to_fill;
        second_c_above->fill_level += to_fill;
      }
    }
  }
  WORLD_MOV(SECOND_WORLD, WORLD);
}

void fluid_event_handle(SDL_Renderer *renderer, SDL_Event *e) {
  static int d;
  static texture texture = TEX_WATER;
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
    }
    if (e->key.keysym.scancode == SDL_SCANCODE_N && !e->key.repeat) {
      fluid_simulate_step(renderer);
    }
  }
}

void fluid_render(SDL_Renderer *renderer) {
  fluid_render_world(renderer);
  fluid_render_grid(renderer);
  fluid_simulate_step(renderer);
}
