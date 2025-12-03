
#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "PerlinNoise.h"

class ProceduralMap {
public:
	static void GenerateWorldMap(sf::Image& worldMap, int setWidth, unsigned int seed = 0);
	static void DisplayWorldMap(sf::RenderWindow& window);

private:
	struct State {
		unsigned int width = 0;
		std::vector<sf::Image> worldLayerProgression;
	};
	static State& GetState();
};

