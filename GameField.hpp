#pragma once 

#include "SFML.hpp"
#include "Randomizer.hpp"
#include <filesystem>
#include <fstream>
#include <utility>


class GameField : public sf::Drawable
{
public:
    GameField(unsigned int fieldWidth, unsigned int fieldHeight, const sf::Vector2f& fieldPosition, unsigned long long randomChance, unsigned long long randomChanceBloody, float cellSize, float cellGap, const sf::Color& aliveCellColor, const sf::Color& bloodyCellColor, const sf::Color& deadCellColor, const sf::Color& hoveredCellColor);

    void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;

    void SetPosition(const sf::Vector2f& position);
    const sf::Vector2f& GetPosition() const;  

    const sf::Vector2u GetSize() const;    

    void Randomize();
    bool Load(const std::string& filePath);
    bool Save(const std::string& filePath) const;
    void Clear();
    void NextGeneration();

    void SetCellSize(float cellSize);
    float GetCellSize() const;

    void SetCellGap(float cellGap);
    float GetCellGap() const;    

    void SetLocalMousePosition(const sf::Vector2u& localMousePosition);
    void MouseClicked();

    unsigned long long GetGeneration() const;

    void SetRandomChance(unsigned long long randomChance);
    unsigned long long GetRandomChance() const;

    void SetAliveCellColor(const sf::Color& aliveCellColor);
    const sf::Color& GetAliveCellColor() const;

	void SetBloodyCellColor(const sf::Color& bloodyCellColor);
	const sf::Color& GetBloodyCellColor() const;

    void SetDeadCellColor(const sf::Color& deadCellColor);
    const sf::Color& GetDeadCellColor() const;

    void SetHoveredCellColor(const sf::Color& hoveredCellColor);
    const sf::Color& GetHoveredCellColor() const;

    bool IsHoveredOnCell() const;
    const sf::Vector2u& GetHoveredCellCoords() const;

    bool IsStable() const;

private:
	// important functions
	unsigned int GetAliveNeighboursCount(const  std::vector<std::vector<unsigned int>>& field, unsigned int x, unsigned int y) const;
	std::pair<unsigned int, unsigned int> SearchForPrey(const  std::vector<std::vector<unsigned int>>& field, unsigned int x, unsigned int y);
    void UpdateVerticles();

private:
    const char FILE_LIVING_CELL_CHAR = 'X';
    std::vector<std::vector<unsigned int>> gameField;
    sf::RectangleShape hoveredCellRect;
    sf::VertexArray verticles;
    sf::Vector2f position;

    bool hoveredOnCell, stable;
    Randomizer randomizer;
    unsigned long long generation, randomChance, randomChanceBloody;
    float cellSize, cellGap, cellSizeAndGap;
    sf::Color aliveCellColor, bloodyCellColor, deadCellColor;
    sf::Vector2u hoveredCellCoords;
};