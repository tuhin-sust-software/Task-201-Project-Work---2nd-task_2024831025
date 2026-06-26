#include <SDL.h>
#include <SDL_ttf.h>
#include <deque>
#include <random>
#include <string>
#include <iostream>

struct Cell {
    int x;
    int y;
};

enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

class Snake {
public:
    Snake(int startX, int startY) {
        body.push_back({startX, startY});
        dir = DIR_RIGHT;
        growPending = 0;
    }

    void setDirection(Direction d) {
        // Prevent reversing directly into yourself
        if ((dir == DIR_UP && d == DIR_DOWN) ||
            (dir == DIR_DOWN && d == DIR_UP) ||
            (dir == DIR_LEFT && d == DIR_RIGHT) ||
            (dir == DIR_RIGHT && d == DIR_LEFT)) {
            return;
        }
        dir = d;
    }

    Direction getDirection() const { return dir; }

    // Move snake by one cell. Return new head position.
    Cell move() {
        Cell head = body.front();
        switch (dir) {
            case DIR_UP:    head.y -= 1; break;
            case DIR_DOWN:  head.y += 1; break;
            case DIR_LEFT:  head.x -= 1; break;
            case DIR_RIGHT: head.x += 1; break;
        }
        body.push_front(head);
        if (growPending > 0) {
            growPending--;
        } else {
            body.pop_back();
        }
        return head;
    }

    void grow(int amount = 1) {
        growPending += amount;
    }

    bool hitsSelf() const {
        if (body.size() < 4) return false;
        const Cell& head = body.front();
        int i = 0;
        for (const auto& c : body) {
            if (i > 0 && c.x == head.x && c.y == head.y) return true;
            i++;
        }
        return false;
    }

    bool occupies(int x, int y) const {
        for (const auto& c : body) {
            if (c.x == x && c.y == y) return true;
        }
        return false;
    }

    const std::deque<Cell>& getBody() const { return body; }

private:
    std::deque<Cell> body;
    Direction dir;
    int growPending;
};

struct Game {
    // Grid config
    const int cellSize = 20;
    const int gridCols = 32; // 32 * 20 = 640
    const int gridRows = 24; // 24 * 20 = 480

    // Timing
    const int tickMs = 120; // snake speed (lower is faster)

    // SDL
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    // RNG
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int> randCol{0, gridCols - 1};
    std::uniform_int_distribution<int> randRow{0, gridRows - 1};

    // Game state
    bool running = true;
    bool gameOver = false;
    Snake snake{gridCols / 2, gridRows / 2};
    Cell food{0, 0};
    int score = 0;

    bool initSDL() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
            std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
            return false;
        }
        if (TTF_Init() != 0) {
            std::cerr << "TTF_Init error: " << TTF_GetError() << "\n";
            return false;
        }

        window = SDL_CreateWindow("Snake (SDL2)",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  gridCols * cellSize, gridRows * cellSize,
                                  SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
            return false;
        }

        font = TTF_OpenFont("DejaVuSans.ttf", 18);
        if (!font) {
            std::cerr << "Warning: could not open font. Score text will be hidden. " << TTF_GetError() << "\n";
        }

        // Start snake slightly longer to look nicer
        snake.grow(3);

        placeFood();

        return true;
    }

    void cleanup() {
        if (font) { TTF_CloseFont(font); font = nullptr; }
        if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
        if (window) { SDL_DestroyWindow(window); window = nullptr; }
        TTF_Quit();
        SDL_Quit();
    }

    void placeFood() {
        // Find a random free cell not occupied by snake
        for (int tries = 0; tries < 1000; ++tries) {
            int x = randCol(rng);
            int y = randRow(rng);
            if (!snake.occupies(x, y)) {
                food = {x, y};
                return;
            }
        }
        // Fallback (should never happen unless grid is full)
        food = {0, 0};
    }

    void handleInput() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                        snake.setDirection(DIR_UP);
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        snake.setDirection(DIR_DOWN);
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        snake.setDirection(DIR_LEFT);
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        snake.setDirection(DIR_RIGHT);
                        break;
                    case SDLK_r:
                        if (gameOver) reset();
                        break;
                }
            }
        }
    }

    void reset() {
        gameOver = false;
        score = 0;
        snake = Snake(gridCols / 2, gridRows / 2);
        snake.grow(3);
        placeFood();
    }

    void update() {
        if (gameOver) return;

        Cell head = snake.move();

        // Boundary collision
        if (head.x < 0 || head.x >= gridCols || head.y < 0 || head.y >= gridRows) {
            gameOver = true;
            return;
        }

        // Self collision
        if (snake.hitsSelf()) {
            gameOver = true;
            return;
        }

        // Food collision
        if (head.x == food.x && head.y == food.y) {
            snake.grow(2);
            score += 10;
            placeFood();
        }
    }

    void drawRectCell(int cx, int cy, SDL_Color col) {
        SDL_Rect r{cx * cellSize, cy * cellSize, cellSize, cellSize};
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &r);
    }

    void drawGrid() {
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        for (int x = 0; x <= gridCols; ++x) {
            SDL_RenderDrawLine(renderer, x * cellSize, 0, x * cellSize, gridRows * cellSize);
        }
        for (int y = 0; y <= gridRows; ++y) {
            SDL_RenderDrawLine(renderer, 0, y * cellSize, gridCols * cellSize, y * cellSize);
        }
    }

    void drawText(const std::string& text, int x, int y, SDL_Color col) {
        if (!font) return;
        SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), col);
        if (!surf) return;
        SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        if (!tex) return;

        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        SDL_Rect dst{x, y, w, h};
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }

    void render() {
        // Clear background
        SDL_SetRenderDrawColor(renderer, 18, 18, 18, 255);
        SDL_RenderClear(renderer);

        // Optional grid (comment out if you want a cleaner look)
        drawGrid();

        // Draw food
        drawRectCell(food.x, food.y, SDL_Color{220, 70, 70, 255});

        // Draw snake (head brighter)
        bool first = true;
        for (const auto& c : snake.getBody()) {
            if (first) {
                drawRectCell(c.x, c.y, SDL_Color{100, 200, 100, 255});
                first = false;
            } else {
                drawRectCell(c.x, c.y, SDL_Color{60, 160, 60, 255});
            }
        }

        // Score
        drawText("Score: " + std::to_string(score), 8, 6, SDL_Color{230, 230, 230, 255});

        // Game over overlay
        if (gameOver) {
            // Darken
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
            SDL_Rect full{0, 0, gridCols * cellSize, gridRows * cellSize};
            SDL_RenderFillRect(renderer, &full);

            drawText("GAME OVER", 200, 180, SDL_Color{255, 80, 80, 255});
            drawText("Final Score: " + std::to_string(score), 200, 220, SDL_Color{230, 230, 230, 255});
            drawText("Press R to Restart, ESC to Quit", 140, 260, SDL_Color{200, 200, 200, 255});
        }

        SDL_RenderPresent(renderer);
    }

    void run() {
        if (!initSDL()) return;

        Uint32 lastTick = SDL_GetTicks();

        while (running) {
            handleInput();

            Uint32 now = SDL_GetTicks();
            if (now - lastTick >= (Uint32)tickMs) {
                update();
                lastTick = now;
            }

            render();

            // Small delay to not burn CPU
            SDL_Delay(1);
        }

        cleanup();
    }
};

int main(int, char**) {
    Game g;
    g.run();
    return 0;
}
