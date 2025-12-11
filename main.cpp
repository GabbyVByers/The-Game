
#include "Game.h"

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

    Game game(window);
    game.generateWorld(100, 50, 20);
    while (window.isOpen()) {
        game.run(window);
    }
    return 0;
}

