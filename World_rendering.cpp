
#include "World.h"

void World::Scrape(sf::Image image) {
    State& s = GetState();
    std::vector<Province>& provinces = s.provinces;
    std::vector<sf::Vertex>& provinceTriangles = s.provinceTriangles;
    std::vector<sf::Vertex>& provinceBorders = s.provinceBorders;
    s.worldMap = image;

    std::vector<ProvinceGeometry::Province>& geoProvinces = ProvinceGeometry::GetProvinces();
    int i = 0;
    for (ProvinceGeometry::Province& geoProvince : geoProvinces) {
        Province province;
        province.numTriangleVertices = geoProvince.triangleVertices.size();
        province.numBorderVertices = geoProvince.borderVertices.size();
        province.startIndexTriangleVertices = provinceTriangles.size();
        province.startIndexBorderVertices = provinceBorders.size();
        for (const sf::Vertex& triangleVert : geoProvince.triangleVertices) {
            provinceTriangles.push_back(triangleVert);
        }
        for (const sf::Vertex& borderVert : geoProvince.borderVertices) {
            provinceBorders.push_back(borderVert);
        }
        province.isCoastal = geoProvince.isCoastal;
        for (int fren : geoProvince.neighbourIndices) {
            province.neighbourIndices.push_back(fren);
        }
        province.keyColor = geoProvince.color;
        province.index = i;
        provinces.push_back(province);
        i++;
    }

    sf::Transform& trasformationMatrix = s.trasformationMatrix;
    trasformationMatrix.scale(sf::Vector2f(0.65f, 0.65f));

    for (int i = 0; i < provinces.size(); i++) {
        Province& province = provinces[i];
        if (province.isCoastal) {
            SetProvinceFillColor(i, sf::Color::Yellow);
        }
    }
}

float World::GetCurrentScale() {
    State& s = GetState();
    sf::Transform& trasformationMatrix = s.trasformationMatrix;

    const float* matrix = trasformationMatrix.getMatrix();
    float scale = std::sqrt(matrix[0] * matrix[0] + matrix[1] * matrix[1]);
    return scale;
}

sf::Vector2i World::GetCurrentTranslation() {
    State& s = GetState();
    sf::Transform& trasformationMatrix = s.trasformationMatrix;

    float tx = trasformationMatrix.getMatrix()[12];
    float ty = trasformationMatrix.getMatrix()[13];

    sf::Vector2i globalPosition = sf::Vector2i((int)tx, (int)ty);
    return globalPosition;
}

void World::Zoom(sf::RenderWindow& window, int delta) {
    State& s = GetState();
    sf::Transform& trasformationMatrix = s.trasformationMatrix;

    sf::Vector2i localMousePos = sf::Mouse::getPosition(window) - GetCurrentTranslation();
    sf::Vector2f mouseBefore = sf::Vector2f(localMousePos.x / GetCurrentScale(), localMousePos.y / GetCurrentScale());
    float factor = 1.0f + (float)delta * 0.05f;
    trasformationMatrix.scale(sf::Vector2f(factor, factor));
    sf::Vector2f mouseAfter = sf::Vector2f(localMousePos.x / GetCurrentScale(), localMousePos.y / GetCurrentScale());
    trasformationMatrix.translate(mouseAfter - mouseBefore);
}

void World::Pan(sf::RenderWindow& window) {
    State& s = GetState();
    sf::Transform& trasformationMatrix = s.trasformationMatrix;

    static sf::Vector2i previousMousePosition;
    sf::Vector2i currentMousePosition = sf::Mouse::getPosition(window);

    sf::Vector2f offset;
    offset.x = (float)currentMousePosition.x - (float)previousMousePosition.x;
    offset.y = (float)currentMousePosition.y - (float)previousMousePosition.y;
    offset /= (float)GetCurrentScale();

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle)) {
        trasformationMatrix.translate(offset);
    }
    previousMousePosition = currentMousePosition;
}

void World::Display(sf::RenderWindow& window) {
    State& s = GetState();
    sf::Transform& trasformationMatrix = s.trasformationMatrix;
    std::vector<sf::Vertex>& provinceTriangles = s.provinceTriangles;
    std::vector<sf::Vertex>& provinceBorders = s.provinceBorders;

    window.draw(&provinceTriangles[0], provinceTriangles.size(), sf::PrimitiveType::Triangles, trasformationMatrix);
    window.draw(&provinceBorders[0], provinceBorders.size(), sf::PrimitiveType::Lines, trasformationMatrix);
}

void World::SetProvinceFillColor(int provIndex, sf::Color color) {
    State& s = GetState();
    std::vector<Province> provinces = s.provinces;
    std::vector<sf::Vertex>& provinceTriangles = s.provinceTriangles;

    Province& province = provinces[provIndex];
    for (int i = 0; i < province.numTriangleVertices; i++) {
        sf::Vertex& vertex = provinceTriangles[i + province.startIndexTriangleVertices];
        vertex.color = color;
    }
}

void World::SetProvinceBorderColor(int provIndex, sf::Color color) {
    State& s = GetState();
    std::vector<Province> provinces = s.provinces;
    std::vector<sf::Vertex>& provinceBorders = s.provinceBorders;

    Province& province = provinces[provIndex];
    for (int i = 0; i < province.numBorderVertices; i++) {
        sf::Vertex& vertex = provinceBorders[i + province.startIndexBorderVertices];
        vertex.color = color;
    }
}



