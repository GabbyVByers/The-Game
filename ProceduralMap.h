
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
	static void GenerateWorldMap() {
		State& s = GetState();

		// Base Perlin Noise
		PerlinNoise::Initialize(s.width);
		PerlinNoise::AddLayer(5, 1.0f);
		sf::Image basePerlinNoise = PerlinNoise::GetSFMLImage();
		s.worldLayerProgression.push_back(basePerlinNoise);

		// Boundary Tapering
		sf::Image boundaryTaperedPerlinNoise;
		boundaryTaperedPerlinNoise.resize({ s.width, s.width });
		for (int i = 0; i < s.width; i++) {
			for (int j = 0; j < s.width; j++) {
				sf::Color taperedColor = basePerlinNoise.getPixel(sf::Vector2u(i, j));
				boundaryTaperedPerlinNoise.setPixel(sf::Vector2u(i, j), taperedColor);
			}
		}
		s.worldLayerProgression.push_back(boundaryTaperedPerlinNoise);

	}

	static void DisplayWorldMap(sf::RenderWindow& window) {
		State& s = GetState();
		for (int i = 0; i < s.worldLayerProgression.size(); i++) {
			sf::Image image = s.worldLayerProgression[i];
			sf::Texture texture = sf::Texture(image);
			sf::Sprite sprite = sf::Sprite(texture);
			sprite.setPosition({ 100.0f + (i * 100.0f) + ((float)s.width * i), 100.0f });
			window.draw(sprite);
		}
	}

private:
	struct State {
		unsigned int width = 500;
		std::vector<sf::Image> worldLayerProgression;
	};

	static State& GetState() {
		static State state;
		return state;
	}
};

