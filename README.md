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
 TTF_Font* font = TTF_OpenFont("arial.ttf", 24); 
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return 1;
    }
    Snake snake;
    SDL_Point food;
    int score = 0;
    bool gameOver = false;
    srand(time(nullptr));

    auto placeFood = [&]() {
        bool valid;
        do {
            valid = true;
            food.x = rand() % GRID_WIDTH;
            food.y = rand() % GRID_HEIGHT;
            for (auto &seg : snake.getBody()) {
                if (seg.x == food.x && seg.y == food.y) {
                    valid = false;
                    break;
                }
            }
        } while (!valid);
    };
    placeFood();

    const Uint32 MOVE_INTERVAL = 150; 
    Uint32 lastMoveTime = SDL_GetTicks(); bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            if (!gameOver)
                snake.handleInput(e);
            if (gameOver && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r)
                snake = Snake();
                score = 0;
                gameOver = false;
                placeFood();
            }
        }
   if (!gameOver) {
            Uint32 now = SDL_GetTicks();
            if (now - lastMoveTime >= MOVE_INTERVAL) {
                bool grow = false;
                if (snake.eatsFood(food)) {
                    grow = true;
                    score += 10;
                    placeFood();
                }
                snake.move(grow);

              
                if (snake.checkWallCollision() || snake.checkSelfCollision()) {
                    gameOver = true;
                }

                lastMoveTime = now;
            }
        }

       
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);   // black background
        SDL_RenderClear(renderer);
   SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
        SDL_Rect foodRect = { food.x * CELL_SIZE, food.y * CELL_SIZE,
                              CELL_SIZE, CELL_SIZE };
        SDL_RenderFillRect(renderer, &foodRect);

        
        snake.render(renderer);

       
        SDL_Color white = {255, 255, 255};
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Texture* scoreTex = renderText(scoreText, font, white, renderer);
        if (scoreTex) {
            SDL_Rect scoreRect = {10, 10, 0, 0};
            SDL_QueryTexture(scoreTex, NULL, NULL, &scoreRect.w, &scoreRect.h);
            SDL_RenderCopy(renderer, scoreTex, NULL, &scoreRect);
            SDL_DestroyTexture(scoreTex);
        }

        
        if (gameOver) {
            // Semi-transparent black background
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
            SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderFillRect(renderer, &overlay);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            
            SDL_Texture* goTex = renderText("GAME OVER", font, white, renderer);
            if (goTex) {
                SDL_Rect goRect;
                SDL_QueryTexture(goTex, NULL, NULL, &goRect.w, &goRect.h);
                goRect.x = (WINDOW_WIDTH  - goRect.w) / 2;
                goRect.y = (WINDOW_HEIGHT - goRect.h) / 2 - 30;
                SDL_RenderCopy(renderer, goTex, NULL, &goRect);
                SDL_DestroyTexture(goTex);
            }
std::string finalScore = "Final Score: " + std::to_string(score);
            SDL_Texture* fsTex = renderText(finalScore, font, white, renderer);
            if (fsTex) {
                SDL_Rect fsRect;
                SDL_QueryTexture(fsTex, NULL, NULL, &fsRect.w, &fsRect.h);
                fsRect.x = (WINDOW_WIDTH  - fsRect.w) / 2;
                fsRect.y = (WINDOW_HEIGHT - fsRect.h) / 2 + 10;
                SDL_RenderCopy(renderer, fsTex, NULL, &fsRect);
                SDL_DestroyTexture(fsTex);
            }

           
            SDL_Texture* hintTex = renderText("Press R to Restart", font, white, renderer);
            if (hintTex) {
                SDL_Rect hintRect;
                SDL_QueryTexture(hintTex, NULL, NULL, &hintRect.w, &hintRect.h);
                hintRect.x = (WINDOW_WIDTH  - hintRect.w) / 2;
                hintRect.y = (WINDOW_HEIGHT - hintRect.h) / 2 + 60;
                SDL_RenderCopy(renderer, hintTex, NULL, &hintRect);
                SDL_DestroyTexture(hintTex);
            }
        }

        SDL_RenderPresent(renderer);
    }

   
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
