
#include "Game.h"

int main() {
    sf::RenderWindow window;
    window.create(sf::VideoMode({ 1920, 1080 }), "Gabby's Risk");
    window.setVerticalSyncEnabled(true);

    Game game(window);
    
    game.generateWorld(window, 2);
    while (window.isOpen()) {
        
        game.run(window);
    }
    return 0;
}

