#include "GameField.hpp"


GameField::GameField(unsigned int fieldWidth, unsigned int fieldHeight, const sf::Vector2f& fieldPosition, unsigned long long randomChance, unsigned long long randomChanceBloody, float cellSize, float cellGap, const sf::Color& aliveCellColor, const sf::Color& bloodyCellColor, const sf::Color& deadCellColor, const sf::Color& hoveredCellColor)
    : position(fieldPosition), randomChance(randomChance), randomChanceBloody(randomChanceBloody), cellSize(cellSize), cellGap(cellGap), aliveCellColor(aliveCellColor), bloodyCellColor(bloodyCellColor), deadCellColor(deadCellColor)
{
    gameField = std::vector<std::vector<unsigned int>> (fieldWidth, std::vector<unsigned int>(fieldHeight));

    if(cellSize > 1.f)
        verticles = sf::VertexArray(sf::PrimitiveType::Quads, fieldWidth * fieldHeight * 4);
    else
        verticles = sf::VertexArray(sf::PrimitiveType::Points, fieldWidth * fieldHeight);
    
    cellSizeAndGap = cellSize + cellGap;
    hoveredCellRect = sf::RectangleShape(sf::Vector2f(cellSize, cellSize));
    hoveredCellRect.setFillColor(hoveredCellColor);
    generation = 0;
    stable = false;

    randomizer.Seed((unsigned long long)time(nullptr));

    UpdateVerticles();
}

void GameField::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(verticles, states);

    if(hoveredOnCell)
    {
        target.draw(hoveredCellRect, states);
    }
}

void GameField::SetPosition(const sf::Vector2f& position)
{
    this->position = position;
}

const sf::Vector2f& GameField::GetPosition() const 
{
    return position;
}

const sf::Vector2u GameField::GetSize() const 
{
    if(gameField.size() > 0)
        return sf::Vector2u((unsigned int)gameField.size(), (unsigned int)gameField[0].size());
    
    return sf::Vector2u(0, 0);
}

void GameField::Randomize()
{
    sf::Vector2u gameFieldSize = GetSize();
	this->Clear();

    for (unsigned int x = 0; x < gameFieldSize.x; x++)
    {
        for (unsigned int y = 0; y < gameFieldSize.y; y++)
        {
            bool alive = randomizer.Random<unsigned long long>(1, randomChance) == 1;
            if (alive)
				gameField[x][y] = 1;
			/* enable for bloody cell randomization
			bool bloody = randomizer.Random<unsigned long long>(1, randomChance * 20) == 1;
			if (bloody)
				gameField[x][y] = 2;
			*/
        }
    }

    generation = 0;
    stable = false;
    UpdateVerticles();
}

bool GameField::Load(const std::string& filePath) 
{
    if(filePath != "" && std::experimental::filesystem::exists(filePath))
    {
        std::vector<std::vector<unsigned int>> values;
        std::ifstream file(filePath);
        std::string line = "";

        while (std::getline(file, line))
        {
            std::vector<unsigned int> valueLine;

            for (const auto& value : line)
                valueLine.push_back(toupper(value) == FILE_LIVING_CELL_CHAR);
            
            values.push_back(valueLine);
        }

        for (unsigned int x = 0; x < gameField.size(); x++)
        {
            for (unsigned int y = 0; y < gameField[0].size(); y++)
            {
                gameField[x][y] = false;
            }
        }
        
        if(values.size() < gameField.size() && values[0].size() < gameField[0].size())
        {
            for(unsigned int x = (unsigned int)gameField.size() / 2 - (unsigned int)values[0].size(), x2 = 0; x < (unsigned int)gameField.size(), x2 < (unsigned int)values[0].size(); x++, x2++)
            {
                for(unsigned int y = (unsigned int)gameField[0].size() / 2 - (unsigned int)values.size(), y2 = 0; y < (unsigned int)gameField[0].size(), y2 < (unsigned int)values.size(); y++, y2++)
                {
                    gameField[x][y] = values[x2][y2];
                }
            }
        }
        else
        {
            for (unsigned int y = 0; y < gameField[0].size(); y++)
            {
                for (unsigned int x = 0; x < gameField.size(); x++)
                {
                    gameField[x][y] = values[y][x];
                }
            }
        }

        generation = 0;
        stable = false;
        UpdateVerticles();
        return true;
    }
    return false;
}

bool GameField::Save(const std::string& filePath) const
{
    if(filePath != "")
    {
        std::ofstream file(filePath);

        for (unsigned int y = 0; y < gameField[0].size(); y++)
        {
            for (unsigned int x = 0; x < gameField.size(); x++)
            {
                file << (gameField[x][y] ? "X" : " ");
            }
            file << "\n";
        }
        file.close();
        return true;
    }
    return false;
}

void GameField::Clear()
{
    for (auto &column : gameField)
        std::fill(column.begin(), column.end(), 0);

    generation = 0;
    stable = false;
    UpdateVerticles();
}

void GameField::NextGeneration()
{
    if(!stable)
    {
        bool changed = false;
        std::vector<std::vector<unsigned int>> tempField(gameField);
		
        for (unsigned int x = 0; x < tempField.size(); x++)
        {
            for (unsigned int y = 0; y < tempField[0].size(); y++)
            {
                int aliveNeighboursCount = GetAliveNeighboursCount(tempField, x, y);
				//bloody cell movement
				if (gameField[x][y] == 2)
				{
					std::pair<unsigned int, unsigned int> moveCoords = SearchForPrey(tempField, x, y);
					if (gameField[moveCoords.first][moveCoords.second] == 1)
					{
						gameField[moveCoords.first][moveCoords.second] = 2;
						changed = true;
					}
					else if (gameField[moveCoords.first][moveCoords.second] == 0)
					{
						gameField[x][y] = 0;
						changed = true;
					}
				}
				//green cell generation
                if(gameField[x][y] == 1 && (aliveNeighboursCount < 2 || aliveNeighboursCount > 3))
                {
                    gameField[x][y] = 0;
                    changed = true;
                }
                else if (gameField[x][y] == 0 && aliveNeighboursCount == 3)
                {
                    gameField[x][y] = 1;
                    changed = true;
                }
				//bloody cell generation
				else if (gameField[x][y] == 1 && aliveNeighboursCount >= 2)
				{
					if (randomizer.Random<unsigned long long>(1, randomChanceBloody) == 1)
					{
						gameField[x][y] = 2;
						changed = true;
					}
				}
            }
        }
        stable = !changed;
        if(!stable)
        {
            generation++;
            UpdateVerticles();
        }
    }
}

void GameField::SetCellSize(float cellSize)
{
    this->cellSize = cellSize;
    cellSizeAndGap = cellSize + cellGap;
    hoveredCellRect.setSize(sf::Vector2f(cellSize, cellSize));
    UpdateVerticles();
}

float GameField::GetCellSize() const
{
    return cellSize;
}

void GameField::SetCellGap(float cellGap)
{
    this->cellGap = cellGap;
    cellSizeAndGap = cellSize + cellGap;
    UpdateVerticles();
}

float GameField::GetCellGap() const
{
    return cellGap;
}

void GameField::SetLocalMousePosition(const sf::Vector2u& localMousePosition)
{
    unsigned int relX = (unsigned int)(localMousePosition.x - position.x);
    unsigned int relY = (unsigned int)(localMousePosition.y - position.y);
    
    unsigned int fieldSizeX = (unsigned int)(gameField.size() * cellSizeAndGap);
    unsigned int fieldSizeY = (unsigned int)(gameField[0].size() * cellSizeAndGap);

    hoveredOnCell = false;
    if(relX < fieldSizeX && relX >= 0 && relY < fieldSizeY && relY >= 0)
    {
        if(relX % (unsigned int)cellSizeAndGap < cellSize && relY % (unsigned int)cellSizeAndGap < cellSize)
        {
            hoveredCellCoords = sf::Vector2u((unsigned int)(relX / cellSizeAndGap), (unsigned int)(relY / cellSizeAndGap));
            hoveredCellRect.setPosition(hoveredCellCoords.x * cellSizeAndGap + position.x, hoveredCellCoords.y * cellSizeAndGap + position.y);
            hoveredOnCell = true;
        }
    }
}

void GameField::MouseClicked()
{
    if(hoveredOnCell)
    {
        gameField[hoveredCellCoords.x][hoveredCellCoords.y] = !gameField[hoveredCellCoords.x][hoveredCellCoords.y];
        stable = false;
        UpdateVerticles();
    }
}

unsigned long long GameField::GetGeneration() const 
{
    return generation;
}

void GameField::SetRandomChance(unsigned long long randomChance)
{
    this->randomChance = randomChance;
}

unsigned long long GameField::GetRandomChance() const
{
    return randomChance;
}

void GameField::SetAliveCellColor(const sf::Color& aliveCellColor)
{
    this->aliveCellColor = aliveCellColor;
    UpdateVerticles();
}

const sf::Color& GameField::GetAliveCellColor() const
{
    return aliveCellColor;
}

void GameField::SetBloodyCellColor(const sf::Color& bloodyCellColor)
{
	this->bloodyCellColor = bloodyCellColor;
	UpdateVerticles();
}

const sf::Color& GameField::GetBloodyCellColor() const
{
	return bloodyCellColor;
}

void GameField::SetDeadCellColor(const sf::Color& deadCellColor)
{
    this->deadCellColor = deadCellColor;
    UpdateVerticles();
}

const sf::Color& GameField::GetDeadCellColor() const
{
    return deadCellColor;
}

void GameField::SetHoveredCellColor(const sf::Color& hoveredCellColor)
{
    hoveredCellRect.setFillColor(hoveredCellColor);
}

const sf::Color& GameField::GetHoveredCellColor() const
{
    return hoveredCellRect.getFillColor();
}

bool GameField::IsHoveredOnCell() const
{
    return hoveredOnCell;
}

const sf::Vector2u& GameField::GetHoveredCellCoords() const 
{
    return hoveredCellCoords;
}

bool GameField::IsStable() const
{
    return stable;
}

unsigned int GameField::GetAliveNeighboursCount(const  std::vector<std::vector<unsigned int>>& field, unsigned int x, unsigned int y) const
{
    int aliveNeighbours = 0;
	int neighbourOffsets[8][2] = { {-1,-1}, {0,-1}, {1,-1}, {-1,0}, {1,0}, {-1,1}, {0,1}, {1,1} };

    for(const auto& currentOffset : neighbourOffsets)
    {
        int xToCheck = x + currentOffset[0];
        int yToCheck = y + currentOffset[1];

        if(xToCheck >= 0 && yToCheck >= 0 && xToCheck < field.size() && yToCheck < field[0].size())
        {
            if(field[xToCheck][yToCheck] == 1)
                aliveNeighbours++;
        }
    }
    return aliveNeighbours;
}
//Bloody Cell Behaviour function:
std::pair<unsigned int, unsigned int> GameField::SearchForPrey(const  std::vector<std::vector<unsigned int>>& field, unsigned int x, unsigned int y) 
{
	int neighbourOffsets[8][2] = { { -1,-1 },{ 0,-1 },{ 1,-1 },{ -1,0 },{ 1,0 },{ -1,1 },{ 0,1 },{ 1,1 } };

	for (const auto& currentOffset : neighbourOffsets)
	{
		int xToCheck = x + currentOffset[0];
		int yToCheck = y + currentOffset[1];

		if (xToCheck >= 0 && yToCheck >= 0 && xToCheck < field.size() && yToCheck < field[0].size())
		{
			if (field[xToCheck][yToCheck] == 1)
				return std::make_pair(xToCheck, yToCheck);
		}
	}
	int randomMoveOffset = randomizer.Random<unsigned long long>(1, 8);
	int xToMove = x + neighbourOffsets[randomMoveOffset][0];
	int yToMove = y + neighbourOffsets[randomMoveOffset][1];
	if (xToMove >= 0 && yToMove >= 0 && xToMove < field.size() && yToMove < field[0].size())
		return std::make_pair(xToMove, yToMove);
	else
		return std::make_pair(x, y);
}


void GameField::UpdateVerticles()
{
    size_t i = 0;

    for (unsigned int x = 0; x < gameField.size(); x++)
    {
        for (unsigned int y = 0; y < gameField[0].size(); y++)
        {
            float curPosX = position.x + x * cellSizeAndGap;
            float curPosY = position.y + y * cellSizeAndGap;

            if (cellSize > 1.f)
            {
                sf::Vertex* quadOffset = &verticles[i * 4];

                for (unsigned int j = 0; j < 4; j++)
                {
                    if(gameField[x][y] == 1)
                        quadOffset[j].color = aliveCellColor;
					else if (gameField[x][y] == 2)
						quadOffset[j].color = bloodyCellColor;
                    else if (gameField[x][y] == 0)
                        quadOffset[j].color = deadCellColor;
                }

                quadOffset[0].position = sf::Vector2f(curPosX, curPosY);
                quadOffset[1].position = sf::Vector2f(curPosX + cellSize, curPosY);
                quadOffset[2].position = sf::Vector2f(curPosX + cellSize, curPosY + cellSize);
                quadOffset[3].position = sf::Vector2f(curPosX, curPosY + cellSize);
            }
            else
            {
                sf::Vertex* vertex = &verticles[i];

                if(gameField[x][y] == 1)
                    vertex->color = aliveCellColor;
				else if (gameField[x][y] == 2)
					vertex->color = bloodyCellColor;
                else if (gameField[x][y] == 0)
                    vertex->color = deadCellColor;

                vertex->position = sf::Vector2f(curPosX, curPosY);
            }
            i++;
        }
    }
}