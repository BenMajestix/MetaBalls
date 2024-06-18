#include "main.h"
#include "ll.h"
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <sys/time.h>

Game game = {.running = 1};

int main() {
  initialize();
  game.framerate = FRAMERATE;
  game.show_circles = 0;
  game.speed = 0.8;
  game.show_interpol = 1;
  struct timeval start;
  struct timeval end;

  game.grid_rec.h = SCREEN_HEIGHT / SCREEN_RES;
  game.grid_rec.w = SCREEN_WIDTH / SCREEN_RES;

  add_circle(400, 100, 80, 1, 2);
  add_circle(500, 600, 70, -1, -3);
  add_circle(100, 200, 90, -2, 2);
  add_circle(400, 400, 80, -1, 1);
  add_circle(200, 300, 60, 1, -3);
  add_circle(600, 100, 80, 2, -2);

  while (game.running) {
    if (gettimeofday(&start, NULL)) {
      perror("first gettimeofday()");
      exit(1);
    }
    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
    SDL_RenderClear(game.renderer);
    handle_input();
    move_block();
    draw_grid();
    if (game.show_circles) {
      draw_block();
    }
    SDL_RenderPresent(game.renderer);

    if (gettimeofday(&end, NULL)) {
      perror("second gettimeofday()");
      exit(1);
    }

    long long diff = timeval_diff(NULL, &end, &start);
    long long frametime = 1000000 / game.framerate;
    // printf("frame: %lld\n max: %lld\n", diff, frametime);
    if (diff < frametime) {
      SDL_Delay((frametime - diff) / 1000);
    }
  }
  terminate(EXIT_SUCCESS);
}

void initialize() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("error: failed to initialize SDL: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  // create the game window
  game.window = SDL_CreateWindow("Metaballs", SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                 SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (!game.window) {
    printf("error: failed to open %d x %d window: %s\n", SCREEN_WIDTH,
           SCREEN_HEIGHT, SDL_GetError());
    terminate(EXIT_FAILURE);
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);

  if (!game.renderer) {
    printf("error: failed to create renderer: %s\n", SDL_GetError());
    terminate(EXIT_FAILURE);
  }
}

void terminate(int exit_code) {
  if (game.renderer) {
    SDL_DestroyRenderer(game.renderer);
  }
  if (game.window) {
    SDL_DestroyWindow(game.window);
  }
  SDL_Quit();
  exit(exit_code);
}

void handle_input() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    // change the game state to not running when close or the esc key is pressed
    // so that the game loop is exited in main
    if (e.type == SDL_QUIT ||
        (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
      game.running = 0;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1) {
      game.show_circles = 1;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F2) {
      game.show_circles = 0;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_2) {
      game.speed += 0.2;
    }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_1) {
      game.speed -= 0.2;
    }
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F3) {
		game.show_interpol = 0;
	}
	if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F4) {
		game.show_interpol = 1;
	}
  }
}

long long timeval_diff(struct timeval *difference, struct timeval *end_time,
                       struct timeval *start_time) {
  struct timeval temp_diff;

  if (difference == NULL) {
    difference = &temp_diff;
  }

  difference->tv_sec = end_time->tv_sec - start_time->tv_sec;
  difference->tv_usec = end_time->tv_usec - start_time->tv_usec;

  /* Using while instead of if below makes the code slightly more robust. */

  while (difference->tv_usec < 0) {
    difference->tv_usec += 1000000;
    difference->tv_sec -= 1;
  }

  return 1000000LL * difference->tv_sec + difference->tv_usec;

} /* timeval_diff() */

void add_circle(int x, int y, int r, int dx, int dy) {
  int last_id;
  if (game.head == NULL) {
    last_id = -1;
  } else {
    last_id = game.head->id;
  }
  game.head = ll_new(game.head);
  game.head->x = x;
  game.head->y = y;
  game.head->r = r;
  game.head->dx = dx;
  game.head->dy = dy;
  game.head->id = last_id + 1;
}

void move_block() {
  handle_collisions();
  circle *b = NULL;
  //printf("speed: %f\n", game.speed);
  ll_foreach(game.head, b) {
    b->x = b->x + b->dx * game.speed;
    b->y = b->y + b->dy * game.speed;
  }
}

void draw_grid() {
  SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
  // SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
  int sw = SCREEN_WIDTH / SCREEN_RES;
  int sh = SCREEN_HEIGHT / SCREEN_RES;

  for (int x = 0; x < SCREEN_WIDTH; x = x + (SCREEN_WIDTH / SCREEN_RES)) {
    for (int y = 0; y < SCREEN_HEIGHT; y = y + (SCREEN_HEIGHT / SCREEN_RES)) {
      int res = 0;
      double tl = function_metaball(x, y);
      double tr = function_metaball(x + sw, y);
      double br = function_metaball(x + sw, y + sh);
      double bl = function_metaball(x, y + sh);
      // oben links
      if (tl >= 1) {
        res += 1;
      }
      // oben rechts
      if (tr >= 1) {
        res += 2;
      }
      // unten rechts
      if (br >= 1) {
        res += 4;
      }
      // unten links
      if (bl >= 1) {
        res += 8;
      }
      double coords[4] = {x, y, sw, sh};
	  if(game.show_interpol) {
		  calc_lines_interpol(res, coords);
	  }
	  else{
		  calc_lines(res, coords);
	  }
    }
  }
}

void handle_collisions() {
  circle *b = NULL;
  ll_foreach(game.head, b) {
	  if(b->id == 2) {
		  //printf("%d %d\n", b->dx, b->x);
	  }
    // oben
    if (b->y - b->r <= 0) {
      b->dy = -b->dy;
	  b->y += b->dy;
    }
    // unten
    else if (b->y + b->r >= SCREEN_HEIGHT) {
      b->dy = -b->dy;
	  b->y += b->dy;
    }
    // rechts
    else if (b->x + b->r >= SCREEN_WIDTH) {
      b->dx = -b->dx;
	  b->x += b->dx;
    }
    // links
    else if (b->x - b->r <= 0) {
      b->dx = -b->dx;
	  b->x += b->dx;
	  //printf("links hit: %d\n", b->x-b->r);
    }
  }
}

void draw_block() {
  circle *b = NULL;
  ll_foreach(game.head, b) {
    aacircleRGBA(game.renderer, b->x, b->y, b->r, 0, 255, 0, 255);
  }
}

double function_metaball(int x, int y) {
  double result = 0;
  circle *c = NULL;
  ll_foreach(game.head, c) {
    result +=
        (SDL_pow(c->r, 2) / (SDL_pow((x - c->x), 2) + SDL_pow((y - c->y), 2)));
  }
  return result;
}

int linear_interpolation(int x1, int y1, int x2, int y2, char coord) {
  double res = 0;
  if (coord == 'x') {
    res = x1 +
          (x2 - x1) * ((1 - function_metaball(x1, y1)) /
                       (function_metaball(x2, y2) - function_metaball(x1, y1)));
  } else if (coord == 'y') {
    res = y1 +
          (y2 - y1) * ((1 - function_metaball(x1, y1)) /
                       (function_metaball(x2, y2) - function_metaball(x1, y1)));
  }
  return res;
}

void calc_lines_interpol(int index, double *coords) {
  if (index == 15 || index == 0) {
    // 0 lines
  } else if (index == 5 || index == 10) {
    // two lines
  } else {
    int hit = 0;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    if (index == 1 || index == 2 || index == 6 || index == 9 || index == 13 ||
        index == 14) {
      if (hit == 0) {
        // x1 = coords[0]+coords[2]/2;
        x1 = side_interpol(coords, 0);
        y1 = coords[1];
        hit++;
      } else {
        // x2 = coords[0]+coords[2]/2;
        x2 = side_interpol(coords, 0);
        y2 = coords[1];
      }
      // top
    }
    if (index == 2 || index == 3 || index == 4 || index == 11 || index == 12 ||
        index == 13) {
      if (hit == 0) {
        x1 = coords[0] + coords[2];
        // y1 = coords[1]+coords[3]/2;
        y1 = side_interpol(coords, 1);
        hit++;
      } else {
        x2 = coords[0] + coords[2];
        // y2 = coords[1]+coords[3]/2;
        y2 = side_interpol(coords, 1);
      }
      // right
    }
    if (index == 4 || index == 6 || index == 7 || index == 9 || index == 8 ||
        index == 11) {
      if (hit == 0) {
        // x1 = coords[0]+coords[2]/2;
        x1 = side_interpol(coords, 2);
        y1 = coords[1] + coords[3];
        hit++;
      } else {
        // x2 = coords[0]+coords[2]/2;
        x2 = side_interpol(coords, 2);
        y2 = coords[1] + coords[3];
      }
      // bottom
    }
    if (index == 1 || index == 3 || index == 7 || index == 8 || index == 12 ||
        index == 14) {
      if (hit == 0) {
        x1 = coords[0];
        // y1 = coords[1]+coords[3]/2;
        y1 = side_interpol(coords, 3);
        hit++;
      } else {
        x2 = coords[0];
        // y2 = coords[1]+coords[3]/2;
        y2 = side_interpol(coords, 3);
      }
      // left
    }
    aalineRGBA(game.renderer, x1, y1, x2, y2, 0, 255, 0, 255);
  }
}

void calc_lines(int index, double *coords) {
	if (index == 15 || index == 0) {
    // 0 lines
  } else if (index == 5 || index == 10) {
    // two lines
  } else {
    int hit = 0;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    if (index == 1 || index == 2 || index == 6 || index == 9 || index == 13 ||
        index == 14) {
      if (hit == 0) {
        x1 = coords[0]+coords[2]/2;
        y1 = coords[1];
        hit++;
      } else {
        x2 = coords[0]+coords[2]/2;
        y2 = coords[1];
      }
      // top
    }
    if (index == 2 || index == 3 || index == 4 || index == 11 || index == 12 ||
        index == 13) {
      if (hit == 0) {
        x1 = coords[0] + coords[2];
        y1 = coords[1]+coords[3]/2;
        hit++;
      } else {
        x2 = coords[0] + coords[2];
        y2 = coords[1]+coords[3]/2;
      }
      // right
    }
    if (index == 4 || index == 6 || index == 7 || index == 9 || index == 8 ||
        index == 11) {
      if (hit == 0) {
        x1 = coords[0]+coords[2]/2;
        y1 = coords[1] + coords[3];
        hit++;
      } else {
        x2 = coords[0]+coords[2]/2;
        y2 = coords[1] + coords[3];
      }
      // bottom
    }
    if (index == 1 || index == 3 || index == 7 || index == 8 || index == 12 ||
        index == 14) {
      if (hit == 0) {
        x1 = coords[0];
        y1 = coords[1]+coords[3]/2;
        hit++;
      } else {
        x2 = coords[0];
        y2 = coords[1]+coords[3]/2;
      }
      // left
    }
    aalineRGBA(game.renderer, x1, y1, x2, y2, 0, 255, 0, 255);
  }
}

int side_interpol(double *coords, int side) {
  switch (side) {
  case 0:
    linear_interpolation(coords[0], coords[1], coords[0] + coords[2], coords[1],
                         'x');
    break;
  case 1:
    linear_interpolation(coords[0] + coords[2], coords[1],
                         coords[0] + coords[2], coords[1] + coords[3], 'y');
    break;
  case 2:
    linear_interpolation(coords[0], coords[1] + coords[3],
                         coords[0] + coords[2], coords[1] + coords[3], 'x');
    break;
  case 3:
    linear_interpolation(coords[0], coords[1], coords[0], coords[1] + coords[3],
                         'y');
    break;

  default:
    break;
  }
}

/*
void renderText() {
  TTF_Font *Sans = TTF_OpenFont("Roboto-Light.ttf", 24);
  SDL_Color White = {255, 255, 255};
  SDL_Surface *surfaceMessage =
      TTF_RenderText_Solid(Sans, "put your text here", White);
  SDL_Texture *Message =
      SDL_CreateTextureFromSurface(game.renderer, surfaceMessage);
  SDL_Rect Message_rect; // create a rect
  Message_rect.x = 0;    // controls the rect's x coordinate
  Message_rect.y = 0;    // controls the rect's y coordinte
  Message_rect.w = 100;  // controls the width of the rect
  Message_rect.h = 100;
  SDL_RenderCopy(game.renderer, Message, NULL, &Message_rect);
  SDL_FreeSurface(surfaceMessage);
  SDL_DestroyTexture(Message);
}
*/
