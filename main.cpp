
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
    std::vector<std::vector<Arrow>> arrows;

    void generateNoise() {

        for (int i = 0; i < (octaves + 2); i++) {
            std::vector<Arrow> row;
            for (int j = 0; j < (octaves + 2); j++) {
                Arrow arrow;
                row.push_back(arrow);
            }
            arrows.push_back(row);
        }

        for (int i = 0; i < (octaves + 2); i++) {
            for (int j = 0; j < (octaves + 2); j++) {
                Arrow arrow;
                arrow.position  = sf::Vector2f(i * epsilon, j * epsilon);
                arrow.direction = RandomNormalDirection();
                arrow.direction.x *= epsilon;
                arrow.direction.y *= epsilon;
                arrows[i][j] = arrow;
            }
        }

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < width; j++) {
                float u = ((float)i / (float)width) + (0.5f / (float)width);
                float v = ((float)j / (float)width) + (0.5f / (float)width);
                sf::Vector2f uvPos = sf::Vector2f(u, v);

                int arrow_i = (unsigned int)(uvPos.x / epsilon);
                int arrow_j = (unsigned int)(uvPos.y / epsilon);

                sf::Vector2f repPosA = sf::Vector2f( arrow_i      * epsilon,  arrow_j      * epsilon);
                sf::Vector2f repPosB = sf::Vector2f((arrow_i + 1) * epsilon,  arrow_j      * epsilon);
                sf::Vector2f repPosC = sf::Vector2f((arrow_i + 1) * epsilon, (arrow_j + 1) * epsilon);
                sf::Vector2f repPosD = sf::Vector2f( arrow_i      * epsilon, (arrow_j + 1) * epsilon);

                sf::Vector2f locPosA = (uvPos - repPosA);
                sf::Vector2f locPosB = (uvPos - repPosB);
                sf::Vector2f locPosC = (uvPos - repPosC);
                sf::Vector2f locPosD = (uvPos - repPosD);

                float dotA = (locPosA.x * arrows[arrow_i    ][arrow_j    ].direction.x) + (locPosA.y * arrows[arrow_i    ][arrow_j    ].direction.y);
                float dotB = (locPosB.x * arrows[arrow_i + 1][arrow_j    ].direction.x) + (locPosB.y * arrows[arrow_i + 1][arrow_j    ].direction.y);
                float dotC = (locPosC.x * arrows[arrow_i + 1][arrow_j + 1].direction.x) + (locPosC.y * arrows[arrow_i + 1][arrow_j + 1].direction.y);
                float dotD = (locPosD.x * arrows[arrow_i    ][arrow_j + 1].direction.x) + (locPosD.y * arrows[arrow_i    ][arrow_j + 1].direction.y);

                float horizFrac = ((uvPos.x - repPosA.x) / epsilon);
                float vertFrac  = ((uvPos.y - repPosA.y) / epsilon);
                float dotAB = (dotA * horizFrac) + (dotB * (1.0f - horizFrac));
                float dotDC = (dotD * horizFrac) + (dotC * (1.0f - horizFrac));
                float dotABDC = (dotAB * vertFrac) + (dotDC * (1.0f - vertFrac));

                float weight = (dotABDC + 1.0f) * 0.5f;
                weight = fmax(0.0f, weight);
                weight = fmin(1.0f, weight);
                noiseImage.setPixel(sf::Vector2u(i, j), sf::Color(255 * weight, 255 * weight, 255 * weight));
            }
        }
    }

    void debugViewNoise(sf::RenderWindow& window) {
        float offset = 150.0f;

        sf::Texture texture(noiseImage);
        sf::Sprite sprite(texture);
        sprite.setPosition(sf::Vector2f(offset, offset));
        window.draw(sprite);

        std::vector<sf::Vertex> boxVertices = {
            sf::Vertex{ sf::Vector2f(offset,         offset        ) },
            sf::Vertex{ sf::Vector2f(offset + width, offset        ) },
            sf::Vertex{ sf::Vector2f(offset + width, offset + width) },
            sf::Vertex{ sf::Vector2f(offset,         offset + width) },
            sf::Vertex{ sf::Vector2f(offset,         offset        ) },
        };
        window.draw(&boxVertices[0], boxVertices.size(), sf::PrimitiveType::LineStrip);

        for (auto& row : arrows) {
            for (Arrow& arrow : row) {
                sf::Vector2f origin = sf::Vector2f(offset, offset);
                origin.x += arrow.position.x * (float)width;
                origin.y += arrow.position.y * (float)width;
                sf::Vector2f dir = arrow.direction * (float)(width);
                DrawArrow(window, origin, dir);
            }
        }
    }

    static void DrawArrow(sf::RenderWindow& window, sf::Vector2f& position, sf::Vector2f& direction, float scale = 20.0f) {
        float theta = SmartArcTan(direction);
        std::vector<sf::Vertex> vertices = {
            sf::Vertex{ position },
            sf::Vertex{ position + direction },
            sf::Vertex{ position + direction },
            sf::Vertex{ position + direction + sf::Vector2f(scale * cos(theta + 2.8f), scale * sin(theta + 2.8f)) },
            sf::Vertex{ position + direction },
            sf::Vertex{ position + direction + sf::Vector2f(scale * cos(theta - 2.8f), scale * sin(theta - 2.8f)) },
        };
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

    static sf::Vector2f RandomNormalDirection() {
        while (true) {
            float x = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
            float y = ((float)rand() / ((float)RAND_MAX * 0.5f)) - 1.0f;
            float distSq = (x * x) + (y * y);
            if (distSq < 1.0f) {
                float dist = sqrt(distSq);
                x /= dist;
                y /= dist;
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

    Perlin perlinLayer(800, 7);

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
        //ImGui::Begin("Gabby's Window");
        //static bool bigGUI = true;
        //if (ImGui::Button("Toggle GUI Size")) {
        //    bigGUI = !bigGUI;
        //    ImGui::GetIO().FontGlobalScale = (bigGUI) ? 2.0f : 1.0f;
        //}
        //ImGui::End();

        perlinLayer.debugViewNoise(window);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

