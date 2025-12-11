
#include "Game.h"

float Game::getCurrentScale() {
	const float* matrix = trasformationMatrix.getMatrix();
	float scale = std::sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1]);
	return scale;
}

sf::Vector2i Game::getCurrentTranslation() {
	float tx = trasformationMatrix.getMatrix()[12];
	float ty = trasformationMatrix.getMatrix()[13];
	sf::Vector2i globalPosition = sf::Vector2i((int)tx, (int)ty);
	return globalPosition;
}

void Game::zoom(sf::RenderWindow& window, int delta) {
	sf::Vector2i localMousePos = sf::Mouse::getPosition(window) - getCurrentTranslation();
	sf::Vector2f mouseBefore = sf::Vector2f(localMousePos.x / getCurrentScale(), localMousePos.y / getCurrentScale());
	float factor = 1.0f + (float)delta * 0.05f;
	trasformationMatrix.scale(sf::Vector2f(factor, factor));
	sf::Vector2f mouseAfter = sf::Vector2f(localMousePos.x / getCurrentScale(), localMousePos.y / getCurrentScale());
	trasformationMatrix.translate(mouseAfter - mouseBefore);
}

void Game::pan(sf::RenderWindow& window) {
	static sf::Vector2i previousMousePosition;
	sf::Vector2i currentMousePosition = sf::Mouse::getPosition(window);
	sf::Vector2f offset;
	offset.x = (float)currentMousePosition.x - (float)previousMousePosition.x;
	offset.y = (float)currentMousePosition.y - (float)previousMousePosition.y;
	offset /= (float)getCurrentScale();
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
		trasformationMatrix.translate(offset);
	}
	previousMousePosition = currentMousePosition;
}

void Game::run(sf::RenderWindow& window) {
	handleEvents(window);
	ImGui::SFML::Update(window, deltaClock.restart());
	pan(window);
	window.clear(sf::Color::Blue);
	window.draw(&provinceTriangles[0], provinceTriangles.size(), sf::PrimitiveType::Triangles, trasformationMatrix);
	window.draw(&provinceBorders[0], provinceBorders.size(), sf::PrimitiveType::Lines, trasformationMatrix);
	gui();
	ImGui::SFML::Render(window);
	window.display();
}

void Game::setProvinceFillColor(int provIndex, sf::Color color) {
	RenderProvince& province = renderProvinces[provIndex];
	for (int i = 0; i < province.numTriangleVertices; i++) {
		sf::Vertex& vertex = provinceTriangles[i + province.startIndexTriangleVertices];
		vertex.color = color;
	}
}

void Game::setProvinceBorderColor(int provIndex, sf::Color color) {
	RenderProvince& province = renderProvinces[provIndex];
	for (int i = 0; i < province.numBorderVertices; i++) {
		sf::Vertex& vertex = provinceBorders[i + province.startIndexBorderVertices];
		vertex.color = color;
	}
}

void Game::selectProvince(sf::Vector2i mousePosition) {
	sf::Vector2i globalPosition = getCurrentTranslation();
	sf::Vector2i localMousePosition = mousePosition - globalPosition;
	sf::Vector2u imageIndex;
	float x = 0.5 * ((float)localMousePosition.x / getCurrentScale());
	float y = 0.5 * ((float)localMousePosition.y / getCurrentScale());
	imageIndex.x = round(x);
	imageIndex.y = round(y);
	sf::Vector2u imageSize = worldMap.getSize();
	if (imageIndex.x >= imageSize.x) {
		return;
	}
	if (imageIndex.y >= imageSize.y) {
		return;
	}
	sf::Color color = worldMap.getPixel(imageIndex);
	int index = -1;
	for (RenderProvince& province : renderProvinces) {
		if (province.color == color) {
			index = province.index;
			break;
		}
	}
	if (index != -1) {
		setProvinceFillColor(index, sf::Color::White);
	}
}

