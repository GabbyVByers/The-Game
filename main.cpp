
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

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Window");
    window.setVerticalSyncEnabled(true);
    
    if (!ImGui::SFML::Init(window))
        assert(false && "Bad ImGui Init");
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().FontGlobalScale = 2.0f;
    ImPlot::CreateContext();
    sf::Clock deltaClock;

    ProceduralMap::GenerateWorldMap();

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
                window.setView(sf::View(visible_area));
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color(20, 20, 40));

        ProceduralMap::DisplayWorldMap(window);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

