
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
	static void GenerateWorldMap(int setWidth = 500, unsigned int seed = 0) {
		State& s = GetState();
		std::vector<sf::Image>& worldLayerProgression = s.worldLayerProgression;
		unsigned int& width = s.width;
		
		worldLayerProgression.clear();
		width = setWidth;

		// Base Perlin Noise
		PerlinNoise::Initialize(width, seed);
		PerlinNoise::AddLayer(05, 0.85f);
		PerlinNoise::AddLayer(20, 0.15f);
		sf::Image basePerlinNoise = PerlinNoise::GetSFMLImage();
		worldLayerProgression.push_back(basePerlinNoise);
		std::vector<std::vector<float>> basePerlinNoiseFloatArray = PerlinNoise::Get2DFloatArray();

		// Boundary Tapering
		sf::Image boundaryTaperedPerlinNoise;
		boundaryTaperedPerlinNoise.resize({ width, width });
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				float weight = basePerlinNoiseFloatArray[i][j];
				float halfWidth = (float)width / 2.0f;
				float dist = sqrt((((float)i - halfWidth) * ((float)i - halfWidth)) + (((float)j - halfWidth) * ((float)j - halfWidth)));
				dist = (halfWidth - dist) / halfWidth;
				dist *= 3.0f;
				dist = fmax(0.0f, dist);
				dist = fmin(1.0f, dist);
				weight *= dist;
				sf::Color taperedColor = sf::Color(255 * weight, 255 * weight, 255 * weight);
				boundaryTaperedPerlinNoise.setPixel(sf::Vector2u(i, j), taperedColor);
			}
		}
		worldLayerProgression.push_back(boundaryTaperedPerlinNoise);

		// And on the 67th day there was Water and Land
		sf::Image waterLandSeparationLayer;
		waterLandSeparationLayer.resize({ width, width });
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < width; j++) {
				unsigned char depth = boundaryTaperedPerlinNoise.getPixel(sf::Vector2u(i, j)).r;
				sf::Color landTypeColor;
				if (depth < 127)
					landTypeColor = sf::Color(0, 0, 255);
				else
					landTypeColor = sf::Color(0, 255, 0);
				waterLandSeparationLayer.setPixel(sf::Vector2u(i, j), landTypeColor);
			}
		}
		worldLayerProgression.push_back(waterLandSeparationLayer);
	}

	static void DisplayWorldMap(sf::RenderWindow& window) {
		State& s = GetState();
		std::vector<sf::Image>& worldLayerProgression = s.worldLayerProgression;
		unsigned int& width = s.width;

		for (int i = 0; i < worldLayerProgression.size(); i++) {
			sf::Image image = worldLayerProgression[i];
			sf::Texture texture = sf::Texture(image);
			sf::Sprite sprite = sf::Sprite(texture);
			sprite.setPosition({ 100.0f + (i * 100.0f) + ((float)width * i), 100.0f });
			window.draw(sprite);
		}
	}

	static sf::Image GetSFMLImage() {
		State& s = GetState();
		std::vector<sf::Image>& worldLayerProgression = s.worldLayerProgression;
		return worldLayerProgression[worldLayerProgression.size() - 1];
	}

private:
	struct State {
		unsigned int width = 0;
		std::vector<sf::Image> worldLayerProgression;
	};

	static State& GetState() {
		static State state;
		return state;
	}
};

