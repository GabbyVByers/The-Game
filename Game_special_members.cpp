
#include "Game.h"

Game::Game(sf::RenderWindow& window) {
	if (!ImGui::SFML::Init(window))
		assert(false && "Bad ImGui Init");
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().FontGlobalScale = 2.0f;
	ImPlot::CreateContext();
}

Game::~Game() {
	ImPlot::DestroyContext();
	ImGui::SFML::Shutdown();
}

