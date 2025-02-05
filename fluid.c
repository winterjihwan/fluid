#include "fluid.h"
#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_scancode.h"
#include <math.h>
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

static void fluid_render_cell(SDL_Renderer *renderer, cell *c) {
  color color = fluid_tex_color(c->texture);
  fluid_color_set(renderer, &color);

  float fill_level = c->fill_level;
  if (c->y > 0 && c->y < CELL_COUNT_H - 1 && c->x > 0 &&
      c->x < CELL_COUNT_W - 1) {
    cell *c_above = &WORLD[c->y * CELL_COUNT_W + c->x];
    cell *c_below = &WORLD[c->y * CELL_COUNT_W + c->x];
    if (c_above->texture == TEX_WATER && c_below->texture == TEX_WATER)
      fill_level = 1.0f;
  }

  int y = c->texture == TEX_WATER ? (c->y + (1 - c->fill_level)) * CELL_SIZE
                                  : c->y * CELL_SIZE;
  int h = c->texture == TEX_WATER ? c->fill_level * CELL_SIZE : CELL_SIZE;
  SDL_RenderFillRect(
      renderer,
      &(SDL_Rect){.x = c->x * CELL_SIZE, .y = y, .w = CELL_SIZE, .h = h});
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

float fluid_stable_state(float flow_level) {
  if (flow_level <= 1) {
    return 1;
  } else if (flow_level < 2 * FLOW_LEVEL_MAX + MAX_COMPRESS) {
    return (FLOW_LEVEL_MAX * FLOW_LEVEL_MAX + flow_level * MAX_COMPRESS) /
           (FLOW_LEVEL_MAX + MAX_COMPRESS);
  } else {
    return (flow_level + MAX_COMPRESS) / 2;
  }
}

void fluid_simulate_step(SDL_Renderer *renderer) {
  WORLD_MOV(WORLD, SECOND_WORLD);

  // Rule #1: Flow below
  for (size_t h = 0; h < CELL_COUNT_H; h++) {
    for (size_t w = 0; w < CELL_COUNT_W; w++) {
      int here = h * CELL_COUNT_W + w;
      cell *c_here = &WORLD[here];
      if (c_here->texture != TEX_WATER)
        continue;

      float to_fill = 0;
      float here_fill = c_here->fill_level;
      if (here_fill <= 0.0f)
        continue;

      int below = (h + 1) * CELL_COUNT_W + w;
      cell *c2_here = &SECOND_WORLD[here];
      cell *c_below = &WORLD[below];
      cell *c2_below = &SECOND_WORLD[below];

      if (below < CELL_COUNT_W * CELL_COUNT_H &&
          c_below->texture == TEX_WATER) {
        float below_fill = c_below->fill_level;
        to_fill = fluid_stable_state(here_fill + below_fill) - below_fill;
        if (to_fill > FLOW_LEVEL_MIN)
          to_fill *= 0.5;
        to_fill = CONSTRAIN(to_fill, 0, fminf(FLOW_LEVEL_MAX, here_fill));

        c2_here->fill_level -= to_fill;
        c2_below->fill_level += to_fill;
        here_fill -= to_fill;
      }

      if (here_fill < 0)
        continue;

      // Rule #2: Flow left and right
      int left = h * CELL_COUNT_W + w - 1;
      int right = h * CELL_COUNT_W + w + 1;
      cell *c_left = &WORLD[left];
      cell *c_right = &WORLD[right];
      cell *c2_left = &SECOND_WORLD[left];
      cell *c2_right = &SECOND_WORLD[right];

      if (w <= 0) {
      }
    }
  }

  /*for (size_t h = 0; h < CELL_COUNT_H; h++) {*/
  /*  for (size_t w = 0; w < CELL_COUNT_W; w++) {*/
  /*    if (w <= 0 || w >= CELL_COUNT_W)*/
  /*      continue;*/
  /**/
  /*    int here = h * CELL_COUNT_W + w;*/
  /*    cell *c_here = &WORLD[here];*/
  /*    if (c_here->texture != TEX_WATER)*/
  /*      continue;*/
  /**/
  /*    float here_fill = c_here->fill_level;*/
  /*    if (here_fill == 0.0f)*/
  /*      continue;*/
  /**/
  /*    int below = (h + 1) * CELL_COUNT_W + w;*/
  /*    if (below < CELL_COUNT_W * CELL_COUNT_H &&*/
  /*        WORLD[below].texture == TEX_WATER &&*/
  /*        WORLD[below].fill_level < 0.975f) {*/
  /*      continue;*/
  /*    }*/
  /**/
  /*    int left = h * CELL_COUNT_W + w - 1;*/
  /*    int right = h * CELL_COUNT_W + w + 1;*/
  /*    cell *c_left = &WORLD[left];*/
  /*    cell *c_right = &WORLD[right];*/
  /*    cell *c2_here = &SECOND_WORLD[here];*/
  /*    cell *c2_left = &SECOND_WORLD[left];*/
  /*    cell *c2_right = &SECOND_WORLD[right];*/
  /**/
  /*    float left_fill = c_left->fill_level;*/
  /*    float right_fill = c_right->fill_level;*/
  /**/
  /*    int can_lfill = c_left->texture != TEX_BUCKET && here_fill >
   * left_fill;*/
  /*    int can_rfill = c_right->texture != TEX_BUCKET && here_fill >
   * right_fill;*/
  /**/
  /*    float to_fill = 0;*/
  /*    float to_lfill = 0;*/
  /*    float to_rfill = 0;*/
  /**/
  /*    if (!can_lfill && !can_rfill)*/
  /*      continue;*/
  /**/
  /*    float how_much;*/
  /*    if (can_lfill && !can_rfill) {*/
  /*      how_much = (here_fill - left_fill) / 2;*/
  /*      to_fill += how_much;*/
  /*      to_lfill += how_much;*/
  /*    } else if (!can_lfill && can_rfill) {*/
  /*      how_much = (here_fill - right_fill) / 2;*/
  /*      to_fill += how_much;*/
  /*      to_rfill += how_much;*/
  /*    } else if (can_lfill && can_rfill) {*/
  /*      how_much = (here_fill - left_fill) / 4;*/
  /*      to_fill += how_much;*/
  /*      to_lfill += how_much;*/
  /**/
  /*      how_much = (here_fill - right_fill) / 4;*/
  /*      to_fill += how_much;*/
  /*      to_rfill += how_much;*/
  /*    }*/
  /**/
  /*    c2_here->fill_level -= to_fill;*/
  /*    c2_left->fill_level += to_lfill;*/
  /*    c2_right->fill_level += to_rfill;*/
  /*  }*/
  /*}*/
  /**/
  /*// Rule #3: Pressurized*/
  /*for (size_t h = 0; h < CELL_COUNT_H; h++) {*/
  /*  for (size_t w = 0; w < CELL_COUNT_W; w++) {*/
  /*    if (h == 0)*/
  /*      continue;*/
  /*    int here = h * CELL_COUNT_W + w;*/
  /*    int above = (h + 1) * CELL_COUNT_W + w;*/
  /**/
  /*    cell *c_here = &WORLD[here];*/
  /*    cell *c_above = &WORLD[above];*/
  /*    if (c_here->texture != TEX_WATER || c_above->texture != TEX_WATER)*/
  /*      continue;*/
  /**/
  /*    float here_fill = c_here->fill_level;*/
  /*    if (here_fill > 1.0f) {*/
  /*      c_above->fill_level += here_fill - 1.0;*/
  /*      c_here->fill_level = 1.0;*/
  /*    }*/
  /*  }*/
  /*}*/
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
