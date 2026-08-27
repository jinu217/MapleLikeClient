#include "Game.h"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        Game game;
        game.run();
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Failed to start game: " << error.what() << '\n';
        return 1;
    }
}
