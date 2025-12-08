
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

class World {
public:

    static void Scrape() {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;
        std::vector<sf::Vertex>& provinceTriangles = s.provinceTriangles;
        std::vector<sf::Vertex>& provinceBorders = s.provinceBorders;

        std::vector<ProvinceGeometry::Province>& geoProvinces = ProvinceGeometry::GetProvinces();
        for (ProvinceGeometry::Province& geoProvince : geoProvinces) {
            Province province;
            province.numTriangleVertices = geoProvince.triangleVertices.size();
            province.numBorderVertices = geoProvince.borderVertices.size();
            province.startIndexTriangleVertices = provinceTriangles.size();
            province.startIndexBorderVertices = provinceBorders.size();
            for (const sf::Vertex& triangleVert : geoProvince.triangleVertices) {
                provinceTriangles.push_back(triangleVert);
            }
            for (const sf::Vertex& borderVert : geoProvince.borderVertices) {
                provinceBorders.push_back(borderVert);
            }
            provinces.push_back(province);
        }

        sf::Transform& trasformationMatrix = s.trasformationMatrix;
        trasformationMatrix.scale(sf::Vector2f(0.65f, 0.65f));
    }

    static float GetCurrentScale() {
        State& s = GetState();
        sf::Transform& trasformationMatrix = s.trasformationMatrix;

        const float* matrix = trasformationMatrix.getMatrix();
        float scale = std::sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1]);
        return scale;
    }

    static sf::Vector2i GetCurrentTranslation() {
        State& s = GetState();
        sf::Transform& trasformationMatrix = s.trasformationMatrix;

        float tx = trasformationMatrix.getMatrix()[12];
        float ty = trasformationMatrix.getMatrix()[13];

        sf::Vector2i globalPosition = sf::Vector2i((int)tx, (int)ty);
        return globalPosition;
    }

    static void Zoom(sf::RenderWindow& window, int delta) {
        State& s = GetState();
        sf::Transform& trasformationMatrix = s.trasformationMatrix;
        
        sf::Vector2i localMousePos = sf::Mouse::getPosition(window) - GetCurrentTranslation();
        sf::Vector2f mouseBefore = sf::Vector2f(localMousePos.x / GetCurrentScale(), localMousePos.y / GetCurrentScale());
        float factor = 1.0f + (float)delta * 0.05f;
        trasformationMatrix.scale(sf::Vector2f(factor, factor));
        sf::Vector2f mouseAfter = sf::Vector2f(localMousePos.x / GetCurrentScale(), localMousePos.y / GetCurrentScale());
        trasformationMatrix.translate(mouseAfter - mouseBefore);
    }

    static void Pan(sf::RenderWindow& window) {
        State& s = GetState();
        sf::Transform& trasformationMatrix = s.trasformationMatrix;

        static sf::Vector2i previousMousePosition;
        sf::Vector2i currentMousePosition = sf::Mouse::getPosition(window);

        sf::Vector2f offset;
        offset.x = (float)currentMousePosition.x - (float)previousMousePosition.x;
        offset.y = (float)currentMousePosition.y - (float)previousMousePosition.y;
        offset /= (float)GetCurrentScale();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
            trasformationMatrix.translate(offset);
        }
        previousMousePosition = currentMousePosition;
    }

    static void Display(sf::RenderWindow& window) {
        State& s = GetState();
        sf::Transform& trasformationMatrix = s.trasformationMatrix;
        std::vector<sf::Vertex>& provinceTriangles = s.provinceTriangles;
        std::vector<sf::Vertex>& provinceBorders = s.provinceBorders;

        window.draw(&provinceTriangles[0], provinceTriangles.size(), sf::PrimitiveType::Triangles, trasformationMatrix);
        window.draw(&provinceBorders[0], provinceBorders.size(), sf::PrimitiveType::Lines, trasformationMatrix);
    }

    static void SetProvinceColor(int provIndex) {
        // todo: implement
    }

    static void SetProvinceBorderColor(int provIndex) {
        // todo: implement
    }

    struct Province {
        bool isCoastal = false;
        std::vector<int> neighbourIndices;
        int startIndexBorderVertices;
        int startIndexTriangleVertices;
        int numBorderVertices;
        int numTriangleVertices;
    };

private:
    struct State {
        std::vector<Province> provinces;
        sf::Transform trasformationMatrix;
        std::vector<sf::Vertex> provinceBorders;
        std::vector<sf::Vertex> provinceTriangles;
    };

    static State& GetState() {
        static State state;
        return state;
    }
};

//// Debug Visual
//sf::Clock deltaClock;
//while (true) {
//    HandleEvents(window);
//    ImGui::SFML::Update(window, deltaClock.restart());
//    window.clear(sf::Color(20, 20, 40));
//    ImGui::Begin("Debugger");
//    if (ImGui::Button("Next")) {
//        break;
//    }
//    ImGui::End();
//    ImGui::SFML::Render(window);
//    window.display();
//}

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
    }
}

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

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
    World::Scrape();

    while (window.isOpen()) {
        HandleEvents(window);

        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color::Blue);

        World::Pan(window);
        World::Display(window);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

