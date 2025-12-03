#pragma once

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "PerlinNoise.h"
#include "ProceduralMap.h"

class ProvinceCracker {
public:
    static void BuildProvinces(sf::Image& worldMap);
    static void CrackProvinces(sf::Image& worldMap);
    static void ItterateProvinceGrowth(sf::RenderWindow* window, sf::Image& worldMap);
    static void ItterateUntilFinished(sf::Image& worldMap);

private:
    struct Province {
        sf::Color color;
        std::vector<sf::Vector2u> corePixels;
        std::vector<sf::Vector2u> marginPixels;
    };
    struct State {
        std::vector<Province> provinces;
    };

    static State& GetState();
    static sf::Color GetUniqueColor();
};

