#include "Game.hpp"
#include "Settings.hpp"

int main()
{
        std::unique_ptr<Game> GoL = std::make_unique<Game>(RES_X, RES_Y, MAX_FPS, RANDOM_CHANCE, BLOODY_CELL_RANDOM_CHANCE, CELL_SIZE, CELL_GAP, CELL_ALIVE_COLOR, CELL_BLOODY_COLOR, CELL_DEAD_COLOR, CELL_HOVERED_COLOR, BACKGROUND_COLOR);
        GoL->Run();
        return 0;
}