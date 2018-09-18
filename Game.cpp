#include "Game.hpp"

Game::Game(unsigned int resX, unsigned int resY, unsigned int maxFPS, unsigned long long randomChance, unsigned long long randomChanceBloody, float cellSize, float cellGap, const sf::Color& aliveCellColor, const sf::Color& bloodyCellColor, const sf::Color& deadCellColor, const sf::Color& hoveredCellColor, const sf::Color& backgroundColor)
    : backgroundColor(backgroundColor)
{
    gameWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode(resX, resY), GAME_TITLE, sf::Style::Close);
    SetMaxFPS(maxFPS);

	gameFont.loadFromFile(CONTENT_PATH + FONT_FILE);

    simulationDelay = 50;
    paused = true;

    gameField = std::make_unique<GameField>((unsigned int)(gameWindow->getSize().x / (cellSize + cellGap)), (unsigned int)((gameWindow->getSize().y - GAMEFIELD_HEIGHT_OFFSET) / (cellSize + cellGap)), sf::Vector2f(0, (float)GAMEFIELD_HEIGHT_OFFSET), randomChance, randomChanceBloody, cellSize, cellGap, aliveCellColor, bloodyCellColor, deadCellColor, hoveredCellColor);

    escapeText.setFont(gameFont);
    escapeText.setString("ESC to exit");
    escapeText.setCharacterSize(CHARACTER_SIZE);
    escapeText.setPosition(TEXT_MARGIN, 0);

    nextGenerationText.setFont(gameFont);
    nextGenerationText.setString("N to advance generation manually");
    nextGenerationText.setCharacterSize(CHARACTER_SIZE);
    nextGenerationText.setPosition(TEXT_MARGIN, (float)CHARACTER_SIZE);

    generationVarText.setFont(gameFont);
	generationVarText.setString("Generation: " + std::to_string(gameField->GetGeneration()));
    generationVarText.setCharacterSize(CHARACTER_SIZE);
    generationVarText.setPosition(TEXT_MARGIN, (float)CHARACTER_SIZE * 2);

    hoveredCellCoordsVarText.setFont(gameFont);
    if(gameField->IsHoveredOnCell())
    {
        sf::Vector2u hoveredCellCoords = gameField->GetHoveredCellCoords();
        hoveredCellCoordsVarText.setString("Hovered cell coords: " + std::to_string(hoveredCellCoords.x) + "," + std::to_string(hoveredCellCoords.y));
    }
    else
        hoveredCellCoordsVarText.setString("Hovered cell coords: None");
    hoveredCellCoordsVarText.setCharacterSize(CHARACTER_SIZE);
    hoveredCellCoordsVarText.setPosition(TEXT_MARGIN, (float)CHARACTER_SIZE * 3);

    delayText.setFont(gameFont);
    delayText.setString("UP/DOWN to increase/decrease delay");
    delayText.setCharacterSize(CHARACTER_SIZE);
    delayText.setPosition(gameWindow->getSize().x / 2 - delayText.getGlobalBounds().width / 2 - TEXT_MARGIN, 0);

    delayVarText.setFont(gameFont);
	delayVarText.setString("Simulation delay: " + std::to_string(simulationDelay) + "ms");
    delayVarText.setCharacterSize(CHARACTER_SIZE);
    delayVarText.setPosition(gameWindow->getSize().x / 2 - delayVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE);

    randomChanceText.setFont(gameFont);
    randomChanceText.setString("LEFT/RIGHT to increase/decrease alive cell chance for randomization");
    randomChanceText.setCharacterSize(CHARACTER_SIZE);
    randomChanceText.setPosition(gameWindow->getSize().x / 2 - randomChanceText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE * 2);

    randomChanceVarText.setFont(gameFont);
	randomChanceVarText.setString("Random chance: 1 out of " + std::to_string(randomChance) + " (" + GetRandomChancePercentage() + "%)");
    randomChanceVarText.setCharacterSize(CHARACTER_SIZE);
    randomChanceVarText.setPosition(gameWindow->getSize().x / 2 - randomChanceVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE * 3);

    clearText.setFont(gameFont);
    clearText.setString("C to clear");
    clearText.setCharacterSize(CHARACTER_SIZE);
    clearText.setPosition(gameWindow->getSize().x / 2 - clearText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE * 4);

    randomizeText.setFont(gameFont);
    randomizeText.setString("R to randomize");
    randomizeText.setCharacterSize(CHARACTER_SIZE);
    randomizeText.setPosition(gameWindow->getSize().x - randomizeText.getGlobalBounds().width - TEXT_MARGIN, 0); 

    pauseText.setFont(gameFont);
    pauseText.setString("P to pause/resume");
    pauseText.setCharacterSize(CHARACTER_SIZE);
    pauseText.setPosition(gameWindow->getSize().x - pauseText.getGlobalBounds().width - TEXT_MARGIN, (float)CHARACTER_SIZE);

    pauseVarText.setFont(gameFont);
    sf::String stateString = paused ? "Paused" : "Playing";
    pauseVarText.setString("State: " + stateString);
    pauseVarText.setCharacterSize(CHARACTER_SIZE);
    pauseVarText.setPosition(gameWindow->getSize().x - pauseText.getGlobalBounds().width - TEXT_MARGIN, (float)CHARACTER_SIZE * 2);

    openSaveText.setFont(gameFont);
    openSaveText.setString("O/S to open/save field from/to file");
    openSaveText.setCharacterSize(CHARACTER_SIZE);
    openSaveText.setPosition(gameWindow->getSize().x - pauseText.getGlobalBounds().width - TEXT_MARGIN, (float)CHARACTER_SIZE * 3);
}

Game::~Game()
{
    paused = true;
    sf::sleep(sf::milliseconds(1));
    simulationThread.detach();

    if(gameWindow != nullptr)
    {
        if(gameWindow->isOpen())
            gameWindow->close();
    }
}

void Game::Run()
{   
    RandomizeField();
    simulationThread = std::thread(&Game::simulationTask, this);
    while(gameWindow->isOpen())
    {
        HandleInput();
        Tick();
        Render();
        sf::sleep(sf::milliseconds(1));
    }
}

void Game::HandleInput()
{
    sf::Event event;

    while(gameWindow->pollEvent(event))
    {   
        if (event.type == sf::Event::MouseMoved)
            gameField->SetLocalMousePosition(sf::Vector2u(event.mouseMove.x, event.mouseMove.y));
        else if (event.type == sf::Event::MouseButtonPressed)
            gameField->MouseClicked();
        else if(event.type == sf::Event::Closed)
            gameWindow->close();
        else if(event.type == sf::Event::KeyPressed)
        {
            switch(event.key.code)
            {
                case sf::Keyboard::Escape:
                    gameWindow->close();
                    break;
                case sf::Keyboard::P:
                    ToggleGameState();
                    break;
                case sf::Keyboard::R:
                    RandomizeField();
                    break;
                case sf::Keyboard::N:
					if (paused)
						NextGeneration();
                    break;
                case sf::Keyboard::Up:
                    IncreaseDelay();
                    break;
                case sf::Keyboard::Down:
                    DecreaseDelay();
                    break;
                case sf::Keyboard::Left:
                    DecreaseRandomChance();
                    break;
                case sf::Keyboard::Right:
                    IncreaseRandomChance();
                    break;
                case sf::Keyboard::O:
                    LoadField();
                    break;
                case sf::Keyboard::S:
                    SaveField();
                    break;
                case sf::Keyboard::C:
                    ClearField();
                    break;
            }
        }
    }
}

void Game::Tick()
{
    if (gameField->IsHoveredOnCell())
    {
        sf::Vector2u hoveredCellCoords = gameField->GetHoveredCellCoords();
		hoveredCellCoordsVarText.setString("Hovered cell coords: " + std::to_string(hoveredCellCoords.x) + "," + std::to_string(hoveredCellCoords.y));
    }
    else    
        hoveredCellCoordsVarText.setString("Hovered cell coords: None");
}

void Game::Render()
{
    gameWindow->clear(backgroundColor);

    gameWindow->draw(escapeText);
    gameWindow->draw(nextGenerationText);
    gameWindow->draw(generationVarText);
    gameWindow->draw(hoveredCellCoordsVarText);
    gameWindow->draw(delayText);
    gameWindow->draw(delayVarText);
    gameWindow->draw(randomChanceText);
    gameWindow->draw(randomChanceVarText);
    gameWindow->draw(clearText);
    gameWindow->draw(randomizeText);
    gameWindow->draw(pauseText);
    gameWindow->draw(pauseVarText);
    gameWindow->draw(openSaveText);

    gameWindow->draw(*gameField);

    gameWindow->display();
}

void Game::ToggleGameState()
{
    paused = !paused;
    sf::String stateString = paused ? "Paused" : "Playing";
    pauseVarText.setString("State: " + stateString);
    pauseVarText.setPosition(gameWindow->getSize().x - pauseText.getGlobalBounds().width - TEXT_MARGIN, (float)CHARACTER_SIZE * 2);
}

void Game::RandomizeField()
{
    gameField->Randomize();
    generationVarText.setString("Generation: 0");
}

void Game::IncreaseDelay()
{
    simulationDelay++;
    delayVarText.setString("Simulation delay: " + std::to_string(simulationDelay) + "ms");
    delayVarText.setPosition(gameWindow->getSize().x / 2 - delayVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE);
}

void Game::DecreaseDelay()
{
    if(simulationDelay > 0)
        simulationDelay--;

    delayVarText.setString("Simulation delay: " + std::to_string(simulationDelay) + "ms");
    delayVarText.setPosition(gameWindow->getSize().x / 2 - delayVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE);
}

void Game::IncreaseRandomChance()
{
    gameField->SetRandomChance(gameField->GetRandomChance() + 1);
    randomChanceVarText.setString("Random chance: 1 out of " + std::to_string(gameField->GetRandomChance()) + " (" + GetRandomChancePercentage() + "%)");
    randomChanceVarText.setPosition(gameWindow->getSize().x / 2 - randomChanceVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE * 3);
}

void Game::DecreaseRandomChance()
{
    if (gameField->GetRandomChance() > 1)
        gameField->SetRandomChance(gameField->GetRandomChance() - 1);
    randomChanceVarText.setString("Random chance: 1 out of " + std::to_string(gameField->GetRandomChance()) + " (" + GetRandomChancePercentage() + "%)");
    randomChanceVarText.setPosition(gameWindow->getSize().x / 2 - randomChanceVarText.getGlobalBounds().width / 2 - TEXT_MARGIN, (float)CHARACTER_SIZE * 3);
}

const std::string Game::GetRandomChancePercentage() const
{
    float percentage = 100.f / gameField->GetRandomChance();
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << percentage;
    return ss.str();
}

void Game::SetMaxFPS(unsigned int maxFPS)
{
    gameWindow->setFramerateLimit(maxFPS);
}

void Game::NextGeneration()
{
    if(!gameField->IsStable())
    {
        gameField->NextGeneration();
		generationVarText.setString("Generation: " + std::to_string(gameField->GetGeneration()));
    }
    else
        generationVarText.setString("Generation: " + std::to_string(gameField->GetGeneration()) + "(stable)");
}

const std::string Game::OpenFileDialog(bool save) const
{
    OPENFILENAME ofn = { sizeof(ofn)};

    char fileName[MAX_PATH] = "";

    ofn.hwndOwner = gameWindow->getSystemHandle();
    ofn.lpstrDefExt = "*.txt";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All (*.*)\0*.*\0Text (*.txt)\0*.txt\0Game of Life (*.gol)\0*.gol\0";
    ofn.nFilterIndex = 1;
    std::string initialDir = R"(.\)" + FIELDS_PATH;
    ofn.lpstrInitialDir = initialDir.c_str();
    if(save)
        ofn.lpstrTitle = "Save Game of Life field";
    else
        ofn.lpstrTitle = "Open Game of Life field";

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(save)
        GetSaveFileName(&ofn);
    else    
        GetOpenFileName(&ofn);

    return fileName;
}

void Game::LoadField()
{
    std::string filePath = OpenFileDialog(false);

    if(!gameField->Load(filePath))
        MessageBoxA(gameWindow->getSystemHandle(), "Error loading field or loading canceled", "Game of life error", 0);
}

void Game::SaveField()
{
    std::string filePath = OpenFileDialog(true);

    if(!gameField->Save(filePath))
        MessageBoxA(gameWindow->getSystemHandle(), "Error saving field or saving canceled", "Game of life error", 0);
}

void Game::ClearField()
{
    gameField->Clear();
}

void Game::simulationTask()
{
    while(true)
    {
        while(!paused)
        {
            lockMutex.lock();
            sf::sleep(sf::milliseconds(simulationDelay));
            NextGeneration();
            lockMutex.unlock();
        }
        sf::sleep(sf::milliseconds(10));
    }
}