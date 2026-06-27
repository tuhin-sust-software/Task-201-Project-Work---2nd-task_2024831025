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
  bool eatsFood(SDL_Point food) {
        return (body[0].x == food.x && body[0].y == food.y);
    }

    void render(SDL_Renderer* renderer) {
    
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);   // dark green
        SDL_Rect headRect = { body[0].x * CELL_SIZE, body[0].y * CELL_SIZE,
                              CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &headRect);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (size_t i = 1; i < body.size(); i++) {
            SDL_Rect rect = { body[i].x * CELL_SIZE, body[i].y * CELL_SIZE,
                              CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    std::vector<SDL_Point>& getBody() { return body; }

private:
    std::vector<SDL_Point> body;
    Direction direction;
    Direction nextDirection;
};
SDL_Texture* renderText(const std::string &message, TTF_Font* font,
                        SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surf) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Snake Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        return 1;
    }

