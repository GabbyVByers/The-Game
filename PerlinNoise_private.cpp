
#include "PerlinNoise.h"

sf::Vector2f PerlinNoise::randomNormalDirection() {
	while (true) {
		float x = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
		float y = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
		float distSq = (x * x) + (y * y);
		if (distSq < 1.0f) {
			float dist = sqrt(distSq);
			x /= dist;
			y /= dist;
			return sf::Vector2f(x, y);
		}
	}
}

float PerlinNoise::dotProduct(const sf::Vector2f& A, const sf::Vector2f& B) {
	return (A.x * B.x) + (A.y * B.y);
}

