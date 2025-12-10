#pragma once

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

class WorldGeneration {
public:

	static void HandleEvents(sf::RenderWindow& window) {
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
                window.setView(sf::View(visible_area));
            }
            //if (const auto& mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
            //    World::Zoom(window, mouseWheelScrolled->delta);
            //}
            //if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            //    if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
            //        std::cout << "here2\n";
            //        World::SelectProvince(sf::Mouse::getPosition(window));
            //    }
            //}
        }
	}

	static void WorldEditior(sf::RenderWindow& window) {
        if (!ImGui::SFML::Init(window))
            assert(false && "Bad ImGui Init");
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().FontGlobalScale = 2.0f;
        ImPlot::CreateContext();
        sf::Clock deltaClock;

        sf::Image worldMap;
        ProceduralMap::GenerateWorldMap(worldMap, 800, 2);
        ProvinceCracker::BuildProvinces(worldMap);
        ProvinceGeometry::BuildGeometry(worldMap);
        World::Scrape(worldMap);

        while (window.isOpen()) {
            HandleEvents(window);

            ImGui::SFML::Update(window, deltaClock.restart());
            window.clear(sf::Color::Blue);

            //World::Pan(window);
            World::Display(window);

            ImGui::SFML::Render(window);
            window.display();
        }
	}
};

