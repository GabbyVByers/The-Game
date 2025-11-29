
#include "PerlinNoise.h"

void PerlinNoise::Initialize(unsigned int imageWidth, unsigned int providedSeed) {
	State& s = GetState();

	static int seed = 1;
	if (providedSeed != 1)
		seed = providedSeed;
	else
		seed++;
	srand(seed);

	s.width = imageWidth;
	s.noiseValueArray.clear();
	for (int i = 0; i < s.width; i++) {
		std::vector<float> row;
		for (int j = 0; j < s.width; j++) {
			float zero = 0.0f;
			row.push_back(zero);
		}
		s.noiseValueArray.push_back(row);
	}
}

void PerlinNoise::AddLayer(unsigned int octaves, float weight) {
	State& s = GetState();

	std::vector<std::vector<Arrow>> arrows;
	for (int i = 0; i < (octaves + 2); i++) {
		std::vector<Arrow> row;
		for (int j = 0; j < (octaves + 2); j++) {
			Arrow arrow;
			row.push_back(arrow);
		}
		arrows.push_back(row);
	}

	float epsilon = 1.0f / (float)(octaves + 1);
	for (int i = 0; i < (octaves + 2); i++) {
		for (int j = 0; j < (octaves + 2); j++) {
			Arrow arrow;
			arrow.position = sf::Vector2f(i * epsilon, j * epsilon);
			arrow.direction = randomNormalDirection();
			arrows[i][j] = arrow;
		}
	}

	for (int i = 0; i < s.width; i++) {
		for (int j = 0; j < s.width; j++) {
			sf::Vector2f worldPos = sf::Vector2f(((float)i / s.width) + (0.5f / s.width), ((float)j / s.width) + (0.5f / s.width));
			unsigned int refIndex_i = (unsigned int)(worldPos.x / epsilon);
			unsigned int refIndex_j = (unsigned int)(worldPos.y / epsilon);

			sf::Vector2f gradientA = arrows[refIndex_i][refIndex_j].direction;
			sf::Vector2f gradientB = arrows[refIndex_i + 1][refIndex_j].direction;
			sf::Vector2f gradientC = arrows[refIndex_i + 1][refIndex_j + 1].direction;
			sf::Vector2f gradientD = arrows[refIndex_i][refIndex_j + 1].direction;

			sf::Vector2f anchorA = arrows[refIndex_i][refIndex_j].position;
			sf::Vector2f anchorB = arrows[refIndex_i + 1][refIndex_j].position;
			sf::Vector2f anchorC = arrows[refIndex_i + 1][refIndex_j + 1].position;
			sf::Vector2f anchorD = arrows[refIndex_i][refIndex_j + 1].position;

			sf::Vector2f localPosA = (worldPos - anchorA);
			sf::Vector2f localPosB = (worldPos - anchorB);
			sf::Vector2f localPosC = (worldPos - anchorC);
			sf::Vector2f localPosD = (worldPos - anchorD);

			float dotA = dotProduct(localPosA / epsilon, gradientA);
			float dotB = dotProduct(localPosB / epsilon, gradientB);
			float dotC = dotProduct(localPosC / epsilon, gradientC);
			float dotD = dotProduct(localPosD / epsilon, gradientD);

			float weightLeftRight = localPosA.x / epsilon;
			float weightUpDown = localPosA.y / epsilon;
			weightLeftRight = (-cos(3.141593f * weightLeftRight) / 2.0f) + 0.5f;
			weightUpDown = (-cos(3.141593f * weightUpDown) / 2.0f) + 0.5f;
			float lerpAB = (dotA * (1.0f - weightLeftRight)) + (dotB * weightLeftRight);
			float lerpDC = (dotD * (1.0f - weightLeftRight)) + (dotC * weightLeftRight);
			float lerpABDC = (lerpAB * (1.0f - weightUpDown)) + (lerpDC * weightUpDown);

			float totalNoise = (lerpABDC + 0.707f) / 1.414f;
			float& currValue = s.noiseValueArray[i][j];
			currValue += totalNoise * weight;
			currValue = fmax(0.0f, currValue);
			currValue = fmin(1.0f, currValue);
		}
	}
}

sf::Image PerlinNoise::GetSFMLImage() {
	State& s = GetState();

	sf::Image noiseImage;
	noiseImage.resize({ s.width, s.width });
	for (int i = 0; i < s.width; i++) {
		for (int j = 0; j < s.width; j++) {
			float weight = s.noiseValueArray[i][j];
			noiseImage.setPixel(sf::Vector2u(i, j), sf::Color(255 * weight, 255 * weight, 255 * weight));
		}
	}
	return noiseImage;
}

void PerlinNoise::DebugDisplay(sf::RenderWindow& window, sf::Vector2f position) {
	sf::Image noiseImage = GetSFMLImage();
	sf::Texture texture(noiseImage);
	sf::Sprite sprite(texture);
	sprite.setPosition(position);
	window.draw(sprite);
}

