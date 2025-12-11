
#include "Game.h"

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

    Game game(window);
    int i = 0;
    while (window.isOpen()) {
        game.generateWorld(800, i++);
        game.run(window);
    }
    return 0;
}

