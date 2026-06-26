#include "Game.h"

int main(int argc, char* argv[])
{
    Game game;

    if(game.init()==false)
    {
        return 0;
    }

    game.run();

    game.close();

    return 0;
}

