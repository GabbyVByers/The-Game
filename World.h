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

class World {
public:
    // Rendering
    static void Scrape(sf::Image image);
    static float GetCurrentScale();
    static sf::Vector2i GetCurrentTranslation();
    static void Zoom(sf::RenderWindow& window, int delta);
    static void Pan(sf::RenderWindow& window);
    static void Display(sf::RenderWindow& window);
    static void SetProvinceFillColor(int provIndex, sf::Color color);
    static void SetProvinceBorderColor(int provIndex, sf::Color color);

    static void SelectProvince(sf::Vector2i mousePosition) {
        State& s = GetState();
        sf::Image& worldMap = s.worldMap;
        std::vector<Province>& provinces = s.provinces;
        int& indexSelectedProvince = s.indexSelectedProvince;

        std::cout << "here";

        sf::Vector2i globalPosition = GetCurrentTranslation();
        sf::Vector2i localMousePosition = mousePosition - globalPosition;
        sf::Vector2u imageIndex;
        imageIndex.x = 0.5 * (localMousePosition.x / GetCurrentScale());
        imageIndex.y = 0.5 * (localMousePosition.y / GetCurrentScale());
        sf::Vector2u imageSize = worldMap.getSize();
        if (imageIndex.x >= imageSize.x) {
            return;
        }
        if (imageIndex.y >= imageSize.y) {
            return;
        }
        std::cout << "inside!\n";
        sf::Color color = worldMap.getPixel(imageIndex);
        for (Province& province : provinces) {
            if (province.keyColor == color) {
                indexSelectedProvince = province.index;
                break;
            }
        }
        SetProvinceFillColor(indexSelectedProvince, sf::Color::White);
    }

    struct Province {
        int index;
        sf::Color keyColor;
        bool isCoastal = false;
        std::vector<int> neighbourIndices;
        int startIndexBorderVertices;
        int startIndexTriangleVertices;
        int numBorderVertices;
        int numTriangleVertices;
    };

private:
    struct State {
        sf::Image worldMap;
        int indexSelectedProvince;
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

