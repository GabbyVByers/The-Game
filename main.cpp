
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

#include "PerlinNoise.h"
#include "ProceduralMap.h"
#include "ProvinceCracker.h"
#include "ProvinceGeometry.h"
#include "World.h"
#include "WorldGeneration.h"

void HandleEvents(sf::RenderWindow& window) {
    while (const std::optional event = window.pollEvent()) {
        ImGui::SFML::ProcessEvent(window, *event);
        if (event->is<sf::Event::Closed>())
            window.close();
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
            window.setView(sf::View(visible_area));
        }
        if (const auto& mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
            World::Zoom(window, mouseWheelScrolled->delta);
        }
        if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                std::cout << "here2\n";
                World::SelectProvince(sf::Mouse::getPosition(window));
            }
        }
    }
}

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

    WorldGeneration::WorldEditior(window);

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

