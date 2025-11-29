
/*

	Lightweight Perlin-Noise Library for SFML 3.0+
	Gabby V. Byers 2025 (MIT Liscense)

*/

#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

class PerlinNoise {
public:
	static void Initialize(unsigned int imageWidth, unsigned int providedSeed = 1);
	static void AddLayer(unsigned int octaves, float weight = 1.0f);
	static sf::Image GetSFMLImage();
	static void DebugDisplay(sf::RenderWindow& window, sf::Vector2f position = sf::Vector2f());

private:
	struct State {
		std::vector<std::vector<float>> noiseValueArray;
		unsigned int width;
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

