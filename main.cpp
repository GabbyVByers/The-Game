
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

class Perlin {
public:
    sf::Image    noiseImage;
    unsigned int width;
    unsigned int octaves;
    float        epsilon;

    Perlin(unsigned int width, unsigned int octaves) {
        this->width   = width;
        this->octaves = octaves;
        noiseImage.resize({width, width});
        epsilon = 1.0f / (float)(octaves + 1);
        generateNoise();
    }

    struct Arrow {
        sf::Vector2f position;
        sf::Vector2f direction;
    };
    std::vector<Arrow> arrows;

    void generateNoise() {
        for (int i = 0; i < (octaves + 2); i++) {
            for (int j = 0; j < (octaves + 2); j++) {
                Arrow arrow;
                arrow.position  = sf::Vector2f(i * epsilon * 1000, j * epsilon * 1000);
                arrow.direction = RandomDirection();
                arrows.push_back(arrow);
            }
        }
    }

    void drawNoise(sf::RenderWindow& window) {
        for (Arrow& arrow : arrows) {
            DrawArrow(window, arrow.position, arrow.direction);
        }
    }

    static void DrawArrow(sf::RenderWindow& window, sf::Vector2f& position, sf::Vector2f& direction, float scale = 20.0f) {
        float theta = SmartArcTan(direction);
        std::vector<sf::Vertex> vertices;
        vertices.push_back(sf::Vertex{ position });
        vertices.push_back(sf::Vertex{ position + direction });
        vertices.push_back(sf::Vertex{ position + direction });
        vertices.push_back(sf::Vertex{ position + direction + sf::Vector2f(scale * cos(theta + 2.8f), scale * sin(theta + 2.8f))});
        vertices.push_back(sf::Vertex{ position + direction });
        vertices.push_back(sf::Vertex{ position + direction + sf::Vector2f(scale * cos(theta - 2.8f), scale * sin(theta - 2.8f)) });
        window.draw(&vertices[0], vertices.size(), sf::PrimitiveType::Lines);
    }

    static float SmartArcTan(sf::Vector2f direction) {
        const float PI = 3.1415927f;
        if ((direction.x >= 0.0f) && (direction.y >= 0.0f))
            return atan(direction.y / direction.x);
        if ((direction.x >= 0.0f) && (direction.y <= 0.0f))
            return (2 * PI) + atan(direction.y / direction.x);
        if ((direction.x <= 0.0f))
            return PI + atan(direction.y / direction.x);
    }

    static sf::Vector2f RandomDirection() {
        while (true) {
            float x = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
            float y = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
            float dist = (x * x) + (y * y);
            if (dist < 1.0f) {
                return sf::Vector2f(x, y);
            }
        }
    }
};

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

    Perlin perlinLayer(500, 20);

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
        ImGui::Begin("Gabby's Window");
        static bool bigGUI = true;
        if (ImGui::Button("Toggle GUI Size")) {
            bigGUI = !bigGUI;
            ImGui::GetIO().FontGlobalScale = (bigGUI) ? 2.0f : 1.0f;
        }
        ImGui::End();

        perlinLayer.drawNoise(window);
        //static float theta = 0.0f;
        //theta += 0.01f;
        //Perlin::DrawArrow(window, sf::Vector2f(500.0f, 500.0f), sf::Vector2f(250.0f * cos(theta), 250.0f * sin(theta)));

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

