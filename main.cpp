
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

    static void ItterateProvinceGrowth(sf::RenderWindow& window, sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            if (province.marginPixels.size() == 0) {
                continue;
            }
            int randMarginIndex = rand() % province.marginPixels.size();
            sf::Vector2u marginPixel = province.marginPixels[randMarginIndex];
            bool isFreeRealEstate = worldMap.getPixel(marginPixel) == sf::Color(0, 255, 0);
            if (!isFreeRealEstate) {
                province.marginPixels.erase(province.marginPixels.begin() + randMarginIndex);
                continue;
            }
            province.corePixels.push_back(marginPixel);
            worldMap.setPixel(marginPixel, province.color);
            province.marginPixels.erase(province.marginPixels.begin() + randMarginIndex);
            std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i(1,  0),
                    sf::Vector2i(0, -1),
                    sf::Vector2i(0,  1)
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

        sf::Texture texture = sf::Texture(worldMap);
        sf::Sprite sprite = sf::Sprite(texture);
        sprite.setPosition(sf::Vector2f(100.0f, 100.0f));
        window.draw(sprite);
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

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Continental Conquest");
    window.setVerticalSyncEnabled(true);
    
    if (!ImGui::SFML::Init(window))
        assert(false && "Bad ImGui Init");
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().FontGlobalScale = 2.0f;
    ImPlot::CreateContext();
    sf::Clock deltaClock;

    ProceduralMap::GenerateWorldMap(800, 2);
    sf::Image worldMap = ProceduralMap::GetSFMLImage();
    ProvinceCracker::CrackProvinces(worldMap);

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

        ProvinceCracker::ItterateProvinceGrowth(window, worldMap);

        ImGui::Begin("Debugger");
        if (ImGui::Button("Save World Map")) {
            worldMap.saveToFile("world_map.png");
        }
        ImGui::End();

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

