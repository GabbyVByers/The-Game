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
    static void CrackProvinces(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;
        provinces.clear();

        int density = 100;
        int mapWidth = worldMap.getSize().x;
        int stride = mapWidth / density;
        for (int i = 0; i < density; i++) {
            for (int j = 0; j < density; j++) {
                sf::Vector2u position = sf::Vector2u(i * stride, j * stride);
                bool isWater = worldMap.getPixel(position) == sf::Color(0, 0, 255);
                if (isWater) {
                    continue;
                }
                sf::Color provColor = GetUniqueColor();
                Province newProvince;
                newProvince.color = provColor;
                newProvince.corePixels.push_back(position);
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i( 1,  0),
                    sf::Vector2i( 0, -1),
                    sf::Vector2i( 0,  1)
                };
                for (const sf::Vector2i& offset : offsets) {
                    sf::Vector2u marginPixel = position;
                    marginPixel.x += offset.x;
                    marginPixel.y += offset.y;
                    bool isFreeRealEstate = worldMap.getPixel(marginPixel) == sf::Color(0, 255, 0);
                    if (!isFreeRealEstate) {
                        continue;
                    }
                    newProvince.marginPixels.push_back(marginPixel);
                }
                worldMap.setPixel(position, provColor);
                provinces.push_back(newProvince);
            }
        }
    }

    static void ItterateProvinceGrowth(sf::RenderWindow* window, sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            if (province.marginPixels.size() == 0) {
                continue;
            }
            std::vector<sf::Vector2u> prunedMarginPixels;
            for (int i = 0; i < province.marginPixels.size(); i++) {
                sf::Vector2u marginPixel = province.marginPixels[i];
                bool isFreeRealEstate = worldMap.getPixel(marginPixel) == sf::Color(0, 255, 0);
                if (isFreeRealEstate) {
                    prunedMarginPixels.push_back(marginPixel);
                }
            }
            province.marginPixels = prunedMarginPixels;
            if (province.marginPixels.size() == 0) {
                continue;
            }
            int randMarginIndex = rand() % province.marginPixels.size();
            sf::Vector2u marginPixel = province.marginPixels[randMarginIndex];
            province.corePixels.push_back(marginPixel);
            worldMap.setPixel(marginPixel, province.color);
            province.marginPixels.erase(province.marginPixels.begin() + randMarginIndex);
            std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i( 1,  0),
                    sf::Vector2i( 0, -1),
                    sf::Vector2i( 0,  1)
            };
            for (sf::Vector2i& offset : offsets) {
                sf::Vector2u newMarginPixel = marginPixel;
                newMarginPixel.x += offset.x;
                newMarginPixel.y += offset.y;
                bool isFreeRealEstate = worldMap.getPixel(newMarginPixel) == sf::Color(0, 255, 0);
                if (!isFreeRealEstate) {
                    continue;
                }
                province.marginPixels.push_back(newMarginPixel);
            }
        }

        if (window != nullptr) {
            sf::Image debugImage = worldMap;
            for (Province& province : provinces) {
                for (sf::Vector2u& marginPixel : province.marginPixels) {
                    debugImage.setPixel(marginPixel, sf::Color(255, 0, 0));
                }
            }
            sf::Texture debugTexture = sf::Texture(debugImage);
            sf::Sprite debugSprite = sf::Sprite(debugTexture);
            debugSprite.setPosition(sf::Vector2f(100.0f, 100.0f));
            window->draw(debugSprite);
        }
    }

    static void ItterateUntilFinished(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        while (true) {
            ItterateProvinceGrowth(nullptr, worldMap);
            bool doneItterating = true;
            for (Province& province : provinces) {
                if (province.marginPixels.size() > 0) {
                    doneItterating = false;
                    break;
                }
            }
            if (doneItterating) {
                break;
            }
        }
    }

private:
    struct Province {
        sf::Color color;
        std::vector<sf::Vector2u> corePixels;
        std::vector<sf::Vector2u> marginPixels;
    };

    static sf::Color GetUniqueColor() {
        State& s = GetState();
        while (true) {
            unsigned char r = rand() % 255;
            unsigned char g = rand() % 255;
            unsigned char b = rand() % 255;
            sf::Color newColor = sf::Color(r, g, b);
            for (Province& prov : s.provinces) {
                if (newColor == prov.color) {
                    continue;
                }
            }
            return newColor;
        }
    }

    struct State {
        std::vector<Province> provinces;
    };
    static State& GetState() {
        static State state;
        return state;
    }
};

