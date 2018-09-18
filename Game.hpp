#pragma once

#include "SFML.hpp"
#include "GameField.hpp"
#include <windows.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>



class Game
{
public:
    Game(unsigned int resX, unsigned int resY, unsigned int maxFPS,unsigned long long randomChance, unsigned long long randomChanceBloody, float cellSize, float cellGap, const sf::Color& aliveCellColor, const sf::Color& bloodyCellColor, const sf::Color& deadCellColor, const sf::Color& hoveredCellColor, const sf::Color& backgroundColor);
	Game(Game const &) = delete;
	void operator=(Game) = delete;
	~Game();

    void Run();

protected:
    void HandleInput();
    void Tick();
    void Render();
    void ToggleGameState();
    void RandomizeField();
    void IncreaseDelay();
    void DecreaseDelay();
    void IncreaseRandomChance();
    void DecreaseRandomChance();
    const std::string GetRandomChancePercentage() const;
    void SetMaxFPS(unsigned int maxFPS);
    void NextGeneration();
    const std::string Game::OpenFileDialog(bool save) const;
    void LoadField();
    void SaveField();
    void ClearField();

private:
	const sf::String CONTENT_PATH = R"(content\)";
    const sf::String FIELDS_PATH = R"(fields\)";
    const sf::String FONT_FILE = "MainFont.ttf";
    const sf::String GAME_TITLE = "Game of Life";
    const float TEXT_MARGIN = 10.f;
    const unsigned int CHARACTER_SIZE = 15u;
    const unsigned int GAMEFIELD_HEIGHT_OFFSET = 80u;

    std::unique_ptr <sf::RenderWindow> gameWindow;
    std::unique_ptr <GameField> gameField;

    bool paused;
    unsigned int simulationDelay;
    sf::Vector2i localMousePosition;
    
    sf::Font gameFont;
    sf::Text escapeText, nextGenerationText, generationVarText, hoveredCellCoordsVarText, delayText, delayVarText, randomChanceText, randomChanceVarText, clearText, randomizeText, pauseText, pauseVarText, openSaveText;


    sf::Color backgroundColor;

    std::thread simulationThread;
    std::mutex lockMutex;
    void simulationTask();
};