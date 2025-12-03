
#include "ProvinceCracker.h"

sf::Color ProvinceCracker::GetUniqueColor() {
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

ProvinceCracker::State& ProvinceCracker::GetState() {
    static State state;
    return state;
}

