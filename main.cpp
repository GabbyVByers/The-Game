
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include "cassert"
#include "iostream"
#include "vector"
#include "string"

#include "PerlinNoise.h"

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

    unsigned int mapWidth = 800;

    PerlinNoise::Initialize(mapWidth, 420);
    PerlinNoise::AddLayer(5, 1.0f);
    //PerlinNoise::AddLayer(20, 0.25f);
    //PerlinNoise::AddLayer(30, 0.05f);
    
    sf::Image perlinImage = PerlinNoise::GetSFMLImage();
    sf::Image mapImage = PerlinNoise::GetSFMLImage();
    for (int i = 0; i < mapWidth; i++) {
        for (int j = 0; j < mapWidth; j++) {
            unsigned char height = perlinImage.getPixel(sf::Vector2u(i, j)).r;
            sf::Color mapPixelColor;
            if (height > 127) {
                mapPixelColor = sf::Color(0, 255, 0);
            }
            if (height <= 127) {
                mapPixelColor = sf::Color(0, 0, 255);
            }
            mapImage.setPixel(sf::Vector2u(i, j), mapPixelColor);
        }
    }
    sf::Texture mapTexture = sf::Texture(mapImage);
    sf::Sprite mapSprite = sf::Sprite(mapTexture);
    mapSprite.setPosition(sf::Vector2f(200.0f + (float)mapWidth, 100.0f));

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

        PerlinNoise::DebugDisplay(window, sf::Vector2f(100.0f, 100.0f));
        window.draw(mapSprite);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

