
#include "imgui.h"
#include "imgui-SFML.h"
#include "implot.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include "iostream"
#include "vector"
#include "string"

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Window");
    window.setVerticalSyncEnabled(true);
    
    if (!ImGui::SFML::Init(window))
        assert(false && "Bad ImGUI Init\n");
    ImGui::GetIO().IniFilename = nullptr;
    ImPlot::CreateContext();
    sf::Clock deltaClock;

    sf::CircleShape circle;
    float circleRadius = 50.0f;

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
        circle.setRadius(circleRadius);
        sf::Vector2f position = sf::Vector2f(window.getSize().x, window.getSize().y);
        circle.setPosition(sf::Vector2f(position.x * 0.5f - circle.getRadius(), position.y * 0.5f - circle.getRadius()));
        window.draw(circle);
        ImGui::Begin("Gabby's Window");
        ImGui::Text("Yippie! :3");
        ImGui::SliderFloat("Circle Radius", &circleRadius, 10.0f, 250.0f);

        static bool bigGUI = false;
        if (ImGui::Button("Toggle GUI Size")) {
            bigGUI = !bigGUI;
            ImGui::GetIO().FontGlobalScale = (bigGUI) ? 2.0f : 1.0f;
        }

        int windowHeight = (bigGUI) ? 450 : 250;
        if (ImPlot::BeginPlot("Test Plot", ImVec2(-1, windowHeight), ImPlotFlags_NoInputs)) {
            static std::vector<float> randomData{ 0.5f };
            float data = randomData[randomData.size() - 1] + (((float)rand() / (float)RAND_MAX) * 0.2f) - 0.1f;
            data = fmax(0.0f, data);
            data = fmin(1.0f, data);
            randomData.push_back(data);
            if (randomData.size() > 99)
                randomData.erase(randomData.begin());
            ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, (double)randomData.size(), ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0, ImGuiCond_Always);
            ImPlot::PlotLine("Random Data", &randomData[0], randomData.size());
            ImPlot::EndPlot();
        }

        ImGui::End();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();
    return 0;
}

