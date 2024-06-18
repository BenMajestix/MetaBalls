#include <SDL.h>

#ifndef MAIN_H_
#define MAIN_H_

#define FRAMERATE 15
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define SCREEN_RES 100
const char *FONT_NAME = "Roboto-Light.ttf";
const int FONT_SIZE = 128;

typedef struct {
  int x, y;
  int r;
  int id;
  int dx;
  int dy;
} circle;

void initialize(void);
void terminate(int exit_code);
void handle_input(void);
long long timeval_diff(struct timeval *difference, struct timeval *end_time,
                       struct timeval *start_time);
void move_block(void);
void draw_block(void);
void draw_grid(void);
void handle_collisions(void);
void change_rate(SDL_KeyCode press);
void add_circle(int x, int y, int r, int dx, int dy);
double function_metaball(int x, int y);
int linear_interpolation(int x1, int y1, int x2, int y2, char coord);
void calc_lines_interpol(int index, double *coords);
void calc_lines(int index, double *coords);
int side_interpol(double *coords, int side);
void renderText(void);

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  int running;
  int framerate;
  circle *head;
  SDL_Rect grid_rec;
  int show_circles;
  int show_interpol;
  double speed;
} Game;

#endif // MAIN_H_
