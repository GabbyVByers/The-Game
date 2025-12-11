
#pragma once

#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include <cassert>
#include <iostream>
#include <vector>
#include <string>

class PerlinNoise {
public:
	static void Initialize(unsigned int imageWidth, unsigned int providedSeed = 0);
	static void AddLayer(unsigned int octaves, float weight = 1.0f);
	static sf::Image GetSFMLImage();
	static std::vector<std::vector<float>> Get2DFloatArray();
	static void DebugDisplay(sf::RenderWindow& window, sf::Vector2f position = sf::Vector2f());

private:
	struct State {
		std::vector<std::vector<float>> noiseValueArray;
		unsigned int width = 0;
	};
	static State& GetState() {
		static State state;
		return state;
	}

	struct Arrow {
		sf::Vector2f position;
		sf::Vector2f direction;
	};
	static sf::Vector2f randomNormalDirection();
	static float dotProduct(const sf::Vector2f& A, const sf::Vector2f& B);
};

