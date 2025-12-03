
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

void HandleEvents(sf::RenderWindow& window) {
    while (const std::optional event = window.pollEvent()) {
        ImGui::SFML::ProcessEvent(window, *event);
        if (event->is<sf::Event::Closed>())
            window.close();
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            sf::FloatRect visible_area({ 0.0f, 0.0f }, sf::Vector2f(resized->size));
            window.setView(sf::View(visible_area));
        }
    }
}

class ProvinceGeometryBuilder {
public:
    static void BuildGeometry(sf::Image& worldMap, sf::RenderWindow& window) {
        FillProvinces(worldMap);
        GetVertices(worldMap);
        SortVertices(window);
    }

    static void FillProvinces(sf::Image& worldMap) {
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

    static void GetVertices(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;
        
        // Edge
        for (Province& province : provinces) {
            for (sf::Vector2u& pixel : province.pixels) {
                sf::Vector2u& currentPosition = pixel;
                sf::Color currentColor = worldMap.getPixel(currentPosition);
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i( 1,  0),
                    sf::Vector2i( 0, -1),
                    sf::Vector2i( 0,  1)
                };
                for (sf::Vector2i offset : offsets) {
                    sf::Vector2u offsetPosition = currentPosition;
                    offsetPosition.x += offset.x;
                    offsetPosition.y += offset.y;
                    sf::Color offsetColor = worldMap.getPixel(offsetPosition);
                    if (currentColor != offsetColor) {
                        sf::Vector2u vertexPosition = currentPosition;
                        vertexPosition.x *= 2;
                        vertexPosition.y *= 2;
                        vertexPosition.x += offset.x;
                        vertexPosition.y += offset.y;
                        province.vertices.push_back(vertexPosition);
                    }
                }
            }
        }

        // Corner
        for (Province& province : provinces) {
            for (sf::Vector2u& pixel : province.pixels) {
                sf::Vector2u& currentPosition = pixel;
                sf::Color currentColor = worldMap.getPixel(currentPosition);
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1, -1),
                    sf::Vector2i(-1,  1),
                    sf::Vector2i( 1, -1),
                    sf::Vector2i( 1,  1)
                };
                for (sf::Vector2i offset : offsets) {
                    sf::Vector2u offsetPosition_A = currentPosition;
                    sf::Vector2u offsetPosition_B = currentPosition;
                    offsetPosition_A.x += offset.x;
                    offsetPosition_B.y += offset.y;
                    sf::Color color_A = worldMap.getPixel(offsetPosition_A);
                    sf::Color color_B = worldMap.getPixel(offsetPosition_B);
                    if (color_A != color_B) {
                        if ((color_A != currentColor) && (color_B != currentColor)) {
                            sf::Vector2u vertexPosition = currentPosition;
                            vertexPosition.x *= 2;
                            vertexPosition.y *= 2;
                            vertexPosition.x += offset.x;
                            vertexPosition.y += offset.y;
                            province.vertices.push_back(vertexPosition);
                        }
                    }
                }
            }
        }
    }

    static void SortVertices(sf::RenderWindow& window) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            std::vector<sf::Vector2u> sortedVertices;
            std::vector<sf::Vector2u>& unsortedVertices = province.vertices;
            sortedVertices.push_back(unsortedVertices[0]);
            unsortedVertices.erase(unsortedVertices.begin());

            float min_x = FLT_MAX;
            float min_y = FLT_MAX;
            for (sf::Vector2u& vert : unsortedVertices) {
                if (vert.y < min_y) {
                    min_y = vert.y;
                }
                if (vert.x < min_x) {
                    min_x = vert.x;
                }
            }

            while (true) {
                sf::Vector2u currentVertex = sortedVertices[sortedVertices.size() - 1];
                float closestDistance = FLT_MAX;
                int indexClosestVertex = 0;
                for (int i = 0; i < unsortedVertices.size(); i++) {
                    sf::Vector2u otherVertex = unsortedVertices[i];
                    float distance = sqrt(pow((float)(currentVertex.x - otherVertex.x), 2.0f) + pow((float)(currentVertex.y - otherVertex.y), 2.0f));
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        indexClosestVertex = i;
                    }
                }
                sortedVertices.push_back(unsortedVertices[indexClosestVertex]);
                unsortedVertices.erase(unsortedVertices.begin() + indexClosestVertex);
                if (unsortedVertices.size() == 0) {
                    break;
                }

                HandleEvents(window);
                window.clear(sf::Color(20, 20, 40));
                std::vector<sf::Vertex> debugVertices;
                for (sf::Vector2u vec : unsortedVertices) {
                    sf::Vector2f pos = sf::Vector2f(vec.x, vec.y);
                    sf::Vertex vert;
                    vert.position = pos;
                    vert.color = sf::Color(255, 255, 255);
                    debugVertices.push_back(vert);
                }

                for (sf::Vertex& vert : debugVertices) {
                    vert.position.x -= min_x;
                    vert.position.y -= min_y;
                    vert.position.x += 2.0f;
                    vert.position.y += 2.0f;
                }

                for (sf::Vertex& vert : debugVertices) {
                    vert.position *= 35.0f;
                }
                
                sf::CircleShape circleSprite;
                circleSprite.setRadius(2.0f);
                sf::Vector2f circlePos = sf::Vector2f(currentVertex.x, currentVertex.y);
                circlePos.x += 2.0f;
                circlePos.y += 2.0f;
                circlePos.x -= min_x;
                circlePos.y -= min_y;
                circlePos *= 35.0f;
                circlePos.x -= 2.0f;
                circlePos.y -= 2.0f;

                circleSprite.setPosition(circlePos);

                window.draw(&debugVertices[0], debugVertices.size(), sf::PrimitiveType::Points);
                window.draw(circleSprite);
                window.display();
            }
            province.vertices = sortedVertices;
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
        std::vector<int> neighbourIndecies;
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
    window.setFramerateLimit(5);

    if (!ImGui::SFML::Init(window))
        assert(false && "Bad ImGui Init");
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().FontGlobalScale = 2.0f;
    ImPlot::CreateContext();
    sf::Clock deltaClock;

    sf::Image worldMap;
    ProceduralMap::GenerateWorldMap(worldMap, 800, 2);
    ProvinceCracker::BuildProvinces(worldMap);
    ProvinceGeometryBuilder::BuildGeometry(worldMap, window);

    while (window.isOpen()) {
        HandleEvents(window);

        ImGui::SFML::Update(window, deltaClock.restart());
        window.clear(sf::Color(20, 20, 40));

        //sf::Texture texture = sf::Texture(worldMap);
        //sf::Sprite sprite = sf::Sprite(texture);
        //sprite.setPosition(sf::Vector2f(100.0f, 100.0f));
        //window.draw(sprite);

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

