#ifndef FLUID_H
#define FLUID_H

#include "SDL_events.h"
#include "SDL_render.h"

#define WINDOW_W 1280
#define WINDOW_H 720
#define CELL_COUNT_W (WINDOW_W / CELL_SIZE)
#define CELL_COUNT_H (WINDOW_H / CELL_SIZE)
#define CELL_SIZE 20

#define FLOW_LEVEL_MAX 1.0
#define FLOW_LEVEL_MIN 0.0001
#define FLOW_SPEED_MAX 1.0
#define MAX_COMPRESS 0.02

typedef struct {
  float r, g, b;
} color;

#define COLOR_WHITE (color){.r = 255, .g = 255, .b = 255}
#define COLOR_GRAY (color){.r = 80, .g = 80, .b = 80}
#define COLOR_BLACK (color){.r = 0, .g = 0, .b = 0}
#define COLOR_LBLUE (color){.r = 173, .g = 216, .b = 230}

#define WORLD_MOV(W1, W2)                                                      \
  do {                                                                         \
    for (size_t p = 0; p < CELL_COUNT_H * CELL_COUNT_W; p++)                   \
      W2[p] = W1[p];                                                           \
  } while (0);

#define CONSTRAIN(v, min, max) ((v < min) ? min : (v > max) ? max : v)

typedef enum {
  TEX_BUCKET,
  TEX_WATER,
} texture;

typedef struct {
  int x, y;
  texture texture;

  // [0, 1]
  float fill_level;
} cell;

void fluid_initialize_static(SDL_Renderer *renderer);
void fluid_render(SDL_Renderer *renderer);
void fluid_event_handle(SDL_Renderer *renderer, SDL_Event *e);

#endif
