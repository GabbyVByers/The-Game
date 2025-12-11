
#include "Game.h"

void Game::handleEvents(sf::RenderWindow& window) {
	while (const std::optional event = window.pollEvent()) {
		ImGui::SFML::ProcessEvent(window, *event);
		if (event->is<sf::Event::Closed>())
			window.close();
		if (const auto* resized = event->getIf<sf::Event::Resized>()) {
			sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
			window.setView(sf::View(visible_area));
		}
		if (const auto& mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
			zoom(window, mouseWheelScrolled->delta);
		}
		if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
			if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
				selectProvince(sf::Mouse::getPosition(window));
			}
		}
	}
}

