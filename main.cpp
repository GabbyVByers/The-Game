
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
        SortVertices();
        RemoveInlineVertices();
        FindCenterOfMass();
        FindFriends(worldMap);
        FindCoastalProvinces(worldMap);
        GetBorderVertices();
        ClipEars(window);
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

        // New Corner
        for (Province& province : provinces) {
            for (sf::Vector2u pixel : province.pixels) {
                sf::Vector2u currentPosition = pixel;
                sf::Color currentColor = worldMap.getPixel(currentPosition);
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1, -1),
                    sf::Vector2i(-1,  1),
                    sf::Vector2i( 1, -1),
                    sf::Vector2i( 1,  1)
                };
                for (sf::Vector2i offset : offsets) {
                    sf::Vector2u offsetPosition = currentPosition;
                    sf::Vector2u position_A = currentPosition;
                    sf::Vector2u position_B = currentPosition;
                    sf::Vector2u position_C = currentPosition;
                    sf::Vector2u position_D = currentPosition;
                    position_B.y += offset.y;
                    position_C.x += offset.x;
                    position_D.x += offset.x;
                    position_D.y += offset.y;
                    sf::Color color_A = worldMap.getPixel(position_A);
                    sf::Color color_B = worldMap.getPixel(position_B);
                    sf::Color color_C = worldMap.getPixel(position_C);
                    sf::Color color_D = worldMap.getPixel(position_D);
                    if (color_A == color_B) {
                        if (color_A != color_D) {
                            if (color_A != color_C) {
                                sf::Vector2u vertexPosition = currentPosition;
                                vertexPosition.x *= 2;
                                vertexPosition.y *= 2;
                                vertexPosition.x += offset.x;
                                vertexPosition.y += offset.y;
                                province.vertices.push_back(vertexPosition);
                            }
                        }
                    }
                    if (color_A == color_C) {
                        if (color_A != color_D) {
                            if (color_A != color_B) {
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

        // Remove Duplicate Vertices
        for (Province& province : provinces) {
            std::vector<sf::Vector2u> prunedVertices;
            for (sf::Vector2u vertex : province.vertices) {
                bool isDuplicate = false;
                for (sf::Vector2u otherVertex : prunedVertices) {
                    if (vertex.x == otherVertex.x) {
                        if (vertex.y == otherVertex.y) {
                            isDuplicate = true;
                        }
                    }
                }
                if (!isDuplicate) {
                    prunedVertices.push_back(vertex);
                }
            }
            province.vertices = prunedVertices;
        }
    }

    static void SortVertices() {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            std::vector<sf::Vector2u> sortedVertices;
            std::vector<sf::Vector2u>& unsortedVertices = province.vertices;
            sortedVertices.push_back(unsortedVertices[0]);
            unsortedVertices.erase(unsortedVertices.begin());

            while (true) {
                if (unsortedVertices.size() == 0) {
                    break;
                }

                sf::Vector2u currentVertex = sortedVertices[sortedVertices.size() - 1];
                float closestDistance = FLT_MAX;
                int indexClosestVertex = 0;
                for (int i = 0; i < unsortedVertices.size(); i++) {
                    sf::Vector2u otherVertex = unsortedVertices[i];
                    if (sortedVertices.size() == 1) {
                        if ((float)otherVertex.y < (float)currentVertex.y) {
                            continue;
                        }
                    }
                    float distance = (((float)currentVertex.x - (float)otherVertex.x) * ((float)currentVertex.x - (float)otherVertex.x)) +
                                     (((float)currentVertex.y - (float)otherVertex.y) * ((float)currentVertex.y - (float)otherVertex.y));
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        indexClosestVertex = i;
                    }
                }
                sortedVertices.push_back(unsortedVertices[indexClosestVertex]);
                unsortedVertices.erase(unsortedVertices.begin() + indexClosestVertex);
            }
            province.vertices = sortedVertices;
        }
    }

    static void RemoveInlineVertices() {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;
        
        for (Province& province : provinces) {
            std::vector<sf::Vector2u> prunedVertices;
            for (int i = 0; i < province.vertices.size(); i++) {
                
                int currIndex = i;
                int prevIndex = i - 1;
                int nextIndex = i + 1;
                if (i == 0)
                    prevIndex = province.vertices.size() - 1;
                if (i == province.vertices.size() - 1)
                    nextIndex = 0;

                sf::Vector2u curr = province.vertices[i];
                sf::Vector2u prev = province.vertices[prevIndex];
                sf::Vector2u next = province.vertices[nextIndex];
                sf::Vector2f A = sf::Vector2f((float)next.x - (float)curr.x, (float)next.y - (float)curr.y);
                sf::Vector2f B = sf::Vector2f((float)prev.x - (float)curr.x, (float)prev.y - (float)curr.y);
                A = A.normalized();
                B = B.normalized();
                float dotProd = (A.x * B.x) + (A.y * B.y);
                if (dotProd > -0.8f) {
                    prunedVertices.push_back(curr);
                }
            }

            province.vertices = prunedVertices;
        }
    }

    static void FindCenterOfMass() {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            float x_sum = 0.0f;
            float y_sum = 0.0f;
            int count = 0;
            for (sf::Vector2u pixel : province.pixels) {
                x_sum += (float)pixel.x * 2;
                y_sum += (float)pixel.y * 2;
                count++;
            }
            sf::Vector2f centerMass = sf::Vector2f(x_sum / count, y_sum / count);
            province.centerMass = centerMass;
        }
    }

    static void FindFriends(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            std::vector<sf::Color> friendColors;
            for (sf::Vector2u pixel : province.pixels) {
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i( 1,  0),
                    sf::Vector2i( 0, -1),
                    sf::Vector2i( 0,  1)
                };
                for (sf::Vector2i offset : offsets) {
                    sf::Vector2u offsetPosition = pixel;
                    offsetPosition.x += offset.x;
                    offsetPosition.y += offset.y;
                    sf::Color offsetColor = worldMap.getPixel(offsetPosition);
                    if (offsetColor != province.color) {
                        friendColors.push_back(offsetColor);
                    }
                }
            }
            for (int i = 0; i < provinces.size(); i++) {
                Province& friendProvince = provinces[i];
                for (sf::Color friendColor : friendColors) {
                    if (friendColor == friendProvince.color) {
                        province.neighbourIndecies.push_back(i);
                        goto doubleContinue;
                    }
                }
            doubleContinue:
                continue;
            }
        }
    }

    static void FindCoastalProvinces(sf::Image& worldMap) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            for (sf::Vector2u pixel : province.pixels) {
                std::vector<sf::Vector2i> offsets = {
                    sf::Vector2i(-1,  0),
                    sf::Vector2i( 1,  0),
                    sf::Vector2i( 0, -1),
                    sf::Vector2i( 0,  1)
                };
                for (sf::Vector2i offset : offsets) {
                    sf::Vector2u offsetPosition = pixel;
                    offsetPosition.x += offset.x;
                    offsetPosition.y += offset.y;
                    sf::Color color = worldMap.getPixel(offsetPosition);
                    if (color == sf::Color(0, 0, 255)) {
                        province.isCoastal = true;
                        goto doubleContinue;
                    }
                }
            }
        doubleContinue:
            continue;
        }
    }

    static void GetBorderVertices() {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {

            std::vector<sf::Vertex> borderVertices;
            for (int i = 0; i < province.vertices.size(); i++) {
                int currIndex = i;
                int nextIndex = i + 1;
                if (nextIndex == province.vertices.size()) {
                    nextIndex = 0;
                }
                sf::Vector2u vec_A = province.vertices[currIndex];
                sf::Vector2u vec_B = province.vertices[nextIndex];
                sf::Vertex vertex_A; vertex_A.position = sf::Vector2f((float)vec_A.x, (float)vec_A.y);
                sf::Vertex vertex_B; vertex_B.position = sf::Vector2f((float)vec_B.x, (float)vec_B.y);
                borderVertices.push_back(vertex_A);
                borderVertices.push_back(vertex_B);
            }
            province.borderVertices = borderVertices;
        }
    }

    static void ClipEars(sf::RenderWindow& window) {
        State& s = GetState();
        std::vector<Province>& provinces = s.provinces;

        for (Province& province : provinces) {
            std::vector<sf::Vertex> polygonVertices;
            for (sf::Vector2u& vec : province.vertices) {
                sf::Vertex vertex;
                vertex.position.x = (float)vec.x;
                vertex.position.y = (float)vec.y;
                polygonVertices.push_back(vertex);
            }

            std::vector<sf::Vertex> triangleVertices;
            int currIndex = 0;

            while (true) {
                int prevIndex = currIndex - 1;
                int nextIndex = currIndex + 1;
                if (prevIndex == -1) {
                    prevIndex = polygonVertices.size() - 1;
                }
                if (nextIndex == polygonVertices.size()) {
                    nextIndex = 0;
                }

                // vvvvv Debug Visual vvvvv
                sf::Clock deltaClock;
                std::vector<sf::Vertex> debugPolygonVertices = polygonVertices;
                std::vector<sf::Vertex> debugTriangleVertices = triangleVertices;
                float min_x = FLT_MAX;
                float min_y = FLT_MAX;
                for (sf::Vertex& vertex : debugPolygonVertices) {
                    if (vertex.position.x < min_x) {
                        min_x = vertex.position.x;
                    }
                    if (vertex.position.y < min_y) {
                        min_y = vertex.position.y;
                    }
                }
                for (sf::Vertex& vertex : debugTriangleVertices) {
                    if (vertex.position.x < min_x) {
                        min_x = vertex.position.x;
                    }
                    if (vertex.position.y < min_y) {
                        min_y = vertex.position.y;
                    }
                }
                std::vector<sf::CircleShape> circles;
                // ^^^^^ Debug Visual ^^^^^

                bool pointInsideTriangleTestPassed = true;
                bool convexVertexTestPassed = true;

                const sf::Vertex& prevVert = polygonVertices[prevIndex];
                const sf::Vertex& currVert = polygonVertices[currIndex];
                const sf::Vertex& nextVert = polygonVertices[nextIndex];

                sf::Vector3f A = sf::Vector3f(prevVert.position.x - currVert.position.x, prevVert.position.y - currVert.position.y, 0.0f);
                sf::Vector3f B = sf::Vector3f(nextVert.position.x - currVert.position.x, nextVert.position.y - currVert.position.y, 0.0f);
                A = A.normalized();
                B = B.normalized();
                sf::Vector3f AxB = A.cross(B);
                float AdotB = A.dot(B);

                if (AxB.z <= 0.0f) {
                    convexVertexTestPassed = false;
                }

                if (AdotB < -0.99) {
                    convexVertexTestPassed = false;
                }

                for (int i = 0; i < polygonVertices.size(); i++) {
                    if (i == prevIndex)
                        continue;
                    if (i == currIndex)
                        continue;
                    if (i == nextIndex)
                        continue;
                    sf::Vertex vertex = polygonVertices[i];
                    sf::Vector2f P = vertex.position;
                    sf::Vector2f A = prevVert.position;
                    sf::Vector2f B = currVert.position;
                    sf::Vector2f C = nextVert.position;

                    float W1 = ((A.x * (C.y - A.y)) + ((P.y - A.y) * (C.x - A.x)) - (P.x * (C.y - A.y))) /
                        (((B.y - A.y) * (C.x - A.x)) - ((B.x - A.x) * (C.y - A.y)));
                    float W2 = (P.y - A.y - (W1 * (B.y - A.y))) /
                        (C.y - A.y);

                    if (W1 > -0.0001f) {
                        if (W2 > 0.0001f) {
                            if ((W1 + W2) < 1.0001f) {
                                pointInsideTriangleTestPassed = false;
                                // vvvvv Debug Visual vvvvv
                                sf::CircleShape circle;
                                circle.setRadius(2.5f);
                                sf::Vector2f circlePos;
                                circlePos = P;
                                circle.setFillColor(sf::Color(255, 100, 100));
                                circlePos.x -= min_x;
                                circlePos.y -= min_y;
                                circlePos.x += 2.0f;
                                circlePos.y += 2.0f;
                                circlePos *= 25.0f;
                                circlePos.x -= 2.5f;
                                circlePos.y -= 2.5f;
                                circle.setPosition(circlePos);
                                circles.push_back(circle);
                                // ^^^^^ Debug Visual ^^^^^
                            }
                        }
                    }
                }

                // vvvvv Debug Visual vvvvv
                for (sf::Vertex& vertex : debugPolygonVertices) {
                    vertex.position.x -= min_x;
                    vertex.position.y -= min_y;
                    vertex.position.x += 2.0f;
                    vertex.position.y += 2.0f;
                }
                for (sf::Vertex& vertex : debugTriangleVertices) {
                    vertex.position.x -= min_x;
                    vertex.position.y -= min_y;
                    vertex.position.x += 2.0f;
                    vertex.position.y += 2.0f;
                }
                for (sf::Vertex& vertex : debugPolygonVertices) {
                    vertex.position *= 25.0f;
                    vertex.color = sf::Color(255, 255, 255);
                }
                for (sf::Vertex& vertex : debugTriangleVertices) {
                    vertex.position *= 25.0f;
                    vertex.color = sf::Color(50, 50, 50);
                }
                for (int i = 0; i < 3; i++) {
                    sf::CircleShape circle;
                    circle.setRadius(2.5f);
                    sf::Vector2f circlePos;
                    if (i == 0) {
                        circlePos = polygonVertices[prevIndex].position;
                        circle.setFillColor(sf::Color(100, 100, 255));
                    }
                    if (i == 1) {
                        circlePos = polygonVertices[currIndex].position;
                        circle.setFillColor(sf::Color(100, 255, 100));
                    }
                    if (i == 2) {
                        circlePos = polygonVertices[nextIndex].position;
                        circle.setFillColor(sf::Color(100, 100, 255));
                    }
                    circlePos.x -= min_x;
                    circlePos.y -= min_y;
                    circlePos.x += 2.0f;
                    circlePos.y += 2.0f;
                    circlePos *= 25.0f;
                    circlePos.x -= 2.5f;
                    circlePos.y -= 2.5f;
                    circle.setPosition(circlePos);
                    circles.push_back(circle);
                }
                while (true) {
                    HandleEvents(window);
                    ImGui::SFML::Update(window, deltaClock.restart());
                    window.clear(sf::Color(20, 20, 40));

                    window.draw(&debugTriangleVertices[0], debugTriangleVertices.size(), sf::PrimitiveType::Triangles);
                    window.draw(&debugPolygonVertices[0], debugPolygonVertices.size(), sf::PrimitiveType::Points);
                    for (sf::CircleShape& circle : circles) {
                        window.draw(circle);
                    }

                    ImGui::Begin("Debugger");
                    if (ImGui::Button("Next")) {
                        break;
                    }
                    ImGui::Text("Point Inside Triangle Test: ");
                    ImGui::SameLine();
                    ImGui::Text((pointInsideTriangleTestPassed) ? "Passed" : "Failed");
                    ImGui::Text("Convex Vertex Test: ");
                    ImGui::SameLine();
                    ImGui::Text((convexVertexTestPassed) ? "Passed" : "Failed");
                    ImGui::End();
                    ImGui::SFML::Render(window);
                    window.display();
                }
                // ^^^^^ Debug Visual ^^^^^

                if (!pointInsideTriangleTestPassed) {
                    currIndex++;
                    continue;
                }

                if (!convexVertexTestPassed) {
                    currIndex++;
                    continue;
                }

                triangleVertices.push_back(prevVert);
                triangleVertices.push_back(currVert);
                triangleVertices.push_back(nextVert);

                polygonVertices.erase(polygonVertices.begin() + currIndex);
                currIndex = 0;

                if (polygonVertices.size() == 3) {
                    triangleVertices.push_back(polygonVertices[0]);
                    triangleVertices.push_back(polygonVertices[1]);
                    triangleVertices.push_back(polygonVertices[2]);
                    break;
                }
            }
        }
    }

private:
    struct Province {
        bool isCoastal = false;
        sf::Color color;
        sf::Vector2f centerMass;
        std::vector<int> neighbourIndecies;
        std::vector<sf::Vector2u> pixels;
        std::vector<sf::Vector2u> vertices;
        std::vector<sf::Vertex> borderVertices;
        std::vector<sf::Vertex> triangleVertices;
    };

    struct State {
        std::vector<Province> provinces;
    };

    static State& GetState() {
        static State state;
        return state;
    }
};

//// Debug Visual
//sf::Clock deltaClock;
//while (true) {
//    HandleEvents(window);
//    ImGui::SFML::Update(window, deltaClock.restart());
//    window.clear(sf::Color(20, 20, 40));
//    ImGui::Begin("Debugger");
//    if (ImGui::Button("Next")) {
//        break;
//    }
//    ImGui::End();
//    ImGui::SFML::Render(window);
//    window.display();
//}

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

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

        sf::Texture texture = sf::Texture(worldMap);
        sf::Sprite sprite = sf::Sprite(texture);
        sprite.setPosition(sf::Vector2f(100.0f, 100.0f));
        window.draw(sprite);

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

