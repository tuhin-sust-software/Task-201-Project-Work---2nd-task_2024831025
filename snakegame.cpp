#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

const int SCREEN_WIDTH  = 600;
const int SCREEN_HEIGHT = 400;
const int BLOCK_SIZE    = 20;
const int MOVE_DELAY    = 120;

class Snake {
public:
    Snake(int startX, int startY, int blockSize) {
        this->blockSize = blockSize;
        dirX = 1;
        dirY = 0;
        SDL_Point head = { startX, startY };
        body.push_back(head);
        body.push_back({ startX - blockSize, startY });
        body.push_back({ startX - 2 * blockSize, startY });
    }

    void changeDirection(int newDirX, int newDirY) {
        if (newDirX == -dirX && newDirY == -dirY) return;
        dirX = newDirX;
        dirY = newDirY;
    }

    void move() {
        SDL_Point newHead;
        newHead.x = body[0].x + dirX * blockSize;
        newHead.y = body[0].y + dirY * blockSize;
        body.insert(body.begin(), newHead);
        body.pop_back();
    }

    void grow() {
        SDL_Point tail = body.back();
        body.push_back(tail);
    }

    bool hasEaten(SDL_Point food) {
        SDL_Point head = body[0];
        return (head.x == food.x && head.y == food.y);
    }

    bool checkSelfCollision() {
        SDL_Point head = body[0];
        for (size_t i = 1; i < body.size(); i++) {
            if (body[i].x == head.x && body[i].y == head.y) return true;
        }
        return false;
    }

    bool checkBoundaryCollision(int screenWidth, int screenHeight) {
        SDL_Point head = body[0];
        if (head.x < 0 || head.x >= screenWidth || head.y < 0 || head.y >= screenHeight) return true;
        return false;
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        for (size_t i = 0; i < body.size(); i++) {
            SDL_Rect rect = { body[i].x, body[i].y, blockSize, blockSize };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_Point getHead() { return body[0]; }
    std::vector<SDL_Point> getBody() { return body; }

private:
    std::vector<SDL_Point> body;
    int dirX, dirY;
    int blockSize;
};

SDL_Point generateFood() {
    SDL_Point food;
    food.x = (rand() % (SCREEN_WIDTH  / BLOCK_SIZE)) * BLOCK_SIZE;
    food.y = (rand() % (SCREEN_HEIGHT / BLOCK_SIZE)) * BLOCK_SIZE;
    return food;
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    if (font == nullptr) return;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface == nullptr) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cout << "TTF_Init Error: " << TTF_GetError() << std::endl;
    }

    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("font.ttf", 24);
    if (font == nullptr) {
        std::cout << "Warning: could not load font.ttf (" << TTF_GetError() << "). The game will still run, just without on-screen text." << std::endl;
    }

    Snake snake(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, BLOCK_SIZE);
    SDL_Point food = generateFood();

    int score = 0;
    bool gameOver = false;
    bool quit = false;

    Uint32 lastMoveTime = SDL_GetTicks();
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            else if (event.type == SDL_KEYDOWN) {
                if (gameOver) {
                    if (event.key.keysym.sym == SDLK_r) {
                        snake = Snake(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, BLOCK_SIZE);
                        food  = generateFood();
                        score = 0;
                        gameOver = false;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
                }
                else {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:    snake.changeDirection(0, -1); break;
                        case SDLK_DOWN:  snake.changeDirection(0, 1);  break;
                        case SDLK_LEFT:  snake.changeDirection(-1, 0); break;
                        case SDLK_RIGHT: snake.changeDirection(1, 0);  break;
                    }
                }
            }
        }

        if (!gameOver) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastMoveTime >= MOVE_DELAY) {
                snake.move();
                lastMoveTime = currentTime;
                if (snake.hasEaten(food)) {
                    snake.grow();
                    score += 10;
                    food = generateFood();
                }
                if (snake.checkSelfCollision() || snake.checkBoundaryCollision(SCREEN_WIDTH, SCREEN_HEIGHT)) {
                    gameOver = true;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (!gameOver) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect foodRect = { food.x, food.y, BLOCK_SIZE, BLOCK_SIZE };
            SDL_RenderFillRect(renderer, &foodRect);
            snake.render(renderer);
            SDL_Color white = { 255, 255, 255, 255 };
            renderText(renderer, font, "Score: " + std::to_string(score), 10, 10, white);
        }
        else {
            SDL_Color red   = { 255, 0, 0, 255 };
            SDL_Color white = { 255, 255, 255, 255 };
            renderText(renderer, font, "GAME OVER", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 40, red);
            renderText(renderer, font, "Final Score: " + std::to_string(score), SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2, white);
            renderText(renderer, font, "Press R to Restart or ESC to Quit", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 2 + 40, white);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (font != nullptr) TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
