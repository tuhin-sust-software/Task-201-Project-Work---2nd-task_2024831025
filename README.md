#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
const int WINDOW_WIDTH  = 640;
const int WINDOW_HEIGHT = 480;

const int CELL_SIZE     = 20;
const int GRID_WIDTH    = WINDOW_WIDTH  / CELL_SIZE;
const int GRID_HEIGHT   = WINDOW_HEIGHT / CELL_SIZE;

enum Direction { UP, DOWN, LEFT, RIGHT };

class Snake {
public:
    Snake() {
       
        body.push_back({GRID_WIDTH/2, GRID_HEIGHT/2});
        body.push_back({GRID_WIDTH/2 - 1, GRID_HEIGHT/2});
        body.push_back({GRID_WIDTH/2 - 2, GRID_HEIGHT/2});
        direction = RIGHT;
        nextDirection = RIGHT;
    }
void handleInput(SDL_Event &e) {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:    if (direction != DOWN)  nextDirection = UP;    break;
                case SDLK_DOWN:  if (direction != UP)    nextDirection = DOWN;  break;
                case SDLK_LEFT:  if (direction != RIGHT) nextDirection = LEFT;  break;
                case SDLK_RIGHT: if (direction != LEFT)  nextDirection = RIGHT; break;
            }
        }
    }

    void move(bool grow) {
  
        direction = nextDirection;
        SDL_Point newHead = body[0];
        switch (direction) {
            case UP:    newHead.y--; break;
            case DOWN:  newHead.y++; break;
            case LEFT:  newHead.x--; break;
            case RIGHT: newHead.x++; break;
        }

        body.insert(body.begin(), newHead);

        if (!grow)
            body.pop_back();
    }

    bool checkWallCollision() {
        SDL_Point head = body[0];
        return (head.x < 0 || head.x >= GRID_WIDTH ||
                head.y < 0 || head.y >= GRID_HEIGHT);
    }
    bool checkSelfCollision() {
        SDL_Point head = body[0];
        for (size_t i = 1; i < body.size(); i++) {
            if (body[i].x == head.x && body[i].y == head.y)
                return true;
        }
        return false;
    }

