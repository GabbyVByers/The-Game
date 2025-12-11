#pragma once

#include "PerlinNoise.h"

struct SystemProvince {
	sf::Color color;
	std::vector<sf::Vector2u> corePixels;
	std::vector<sf::Vector2u> marginPixels;
	bool isCoastal = false;
	sf::Vector2f centerMass;
	std::vector<int> neighbourIndices;
	std::vector<sf::Vector2u> pixels;
	std::vector<sf::Vector2u> vertices;
	std::vector<sf::Vertex> borderVertices;
	std::vector<sf::Vertex> triangleVertices;
};

struct RenderProvince {
	int index;
	sf::Color color;
	bool isCoastal = false;
	std::vector<int> neighbourIndices;
	int startIndexBorderVertices;
	int startIndexTriangleVertices;
	int numBorderVertices;
	int numTriangleVertices;
};

class Game {
public:
	// World Generation
	sf::Image worldMap;
	std::vector<SystemProvince> systemProvinces;

	// World Rendering
	sf::Clock deltaClock;
	sf::Transform trasformationMatrix;
	std::vector<sf::Vertex> provinceBorders;
	std::vector<sf::Vertex> provinceTriangles;
	std::vector<RenderProvince> renderProvinces;

	// Special Members
	Game(sf::RenderWindow& window);
	~Game();

	// World Generation
	void saveWorldMapImage();
	void generateWorld(int mapWidth, int density, unsigned int seed);

	// Rendering
	float getCurrentScale();
	sf::Vector2i getCurrentTranslation();
	void zoom(sf::RenderWindow& window, int delta);
	void pan(sf::RenderWindow& window);
	void run(sf::RenderWindow& window);
	void setProvinceFillColor(int provIndex, sf::Color color);
	void setProvinceBorderColor(int provIndex, sf::Color color);
	void selectProvince(sf::Vector2i mousePosition);

	// Events
	void handleEvents(sf::RenderWindow& window);

	// User Interface
	void gui();
};

