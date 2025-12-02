
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

class ProvinceGeometryBuilder {
public:
    static void FloodFillProvinces(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        provinces.clear();
        unsigned int width = worldMap.getSize().x;
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < width; j++) {
                Province newProvince;
                sf::Vector2u currentPosition = sf::Vector2u(i, j);
                sf::Color currentColor = worldMap.getPixel(currentPosition);
                if (currentColor == sf::Color(0, 255, 0))
                    continue;
                if (currentColor == sf::Color(0, 0, 255))
                    continue;
                for (Province& province : provinces) {
                    if (province.color == currentColor) {
                        province.pixels.push_back(currentPosition);
                        goto nestedContinue;
                    }
                }
                newProvince.color = currentColor;
                newProvince.pixels.push_back(currentPosition);
                provinces.push_back(newProvince);
            nestedContinue:
                continue;
            }
        }
    }

    static void DebugVisualize(sf::RenderWindow& window, sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;
        
        static int index = 0;
        if (index >= provinces.size()) {
            index = 0;
        }

        Province& province = provinces[index++];
        for (sf::Vector2u pixel : province.pixels) {
            worldMap.setPixel(pixel, sf::Color(255, 0, 0));
        }

        sf::Texture texture = sf::Texture(worldMap);
        sf::Sprite sprite = sf::Sprite(texture);
        sprite.setPosition(sf::Vector2f(100.0f, 100.0f));
        window.draw(sprite);
    }

private:
    struct Province {
        sf::Color color;
        std::vector<sf::Vector2u> pixels;
        std::vector<sf::Vector2u> vertices;
    };

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
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(10);

    if (!ImGui::SFML::Init(window))
        assert(false && "Bad ImGui Init");
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().FontGlobalScale = 2.0f;
    ImPlot::CreateContext();
    sf::Clock deltaClock;

    ProceduralMap::GenerateWorldMap(800, 2);
    sf::Image worldMap = ProceduralMap::GetSFMLImage();
    ProvinceCracker::CrackProvinces(worldMap);
    ProvinceCracker::ItterateUntilFinished(worldMap);
    ProvinceGeometryBuilder::FloodFillProvinces(worldMap);

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

        sf::Texture texture = sf::Texture(worldMap);
        sf::Sprite sprite = sf::Sprite(texture);
        sprite.setPosition(sf::Vector2f(100.0f, 100.0f));
        window.draw(sprite);

        ProvinceGeometryBuilder::DebugVisualize(window, worldMap);

        ImGui::Begin("Debugger");
        if (ImGui::Button("Save World Map")) {
            if (worldMap.saveToFile("world_map.png")) {
                std::cout << "Image Saved Successfully\n";
            }
        }
        ImGui::End();

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

