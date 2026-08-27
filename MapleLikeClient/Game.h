#pragma once

#include <SFML/Graphics.hpp>

#include "CameraController.h"
#include "LevelData.h"
#include "Inventory.h"
#include "Player.h"
#include "World.h"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void renderHud();
    void renderInventory();
    void restartFromCheckpoint();

    static constexpr sf::Vector2f ViewSize{ 1280.f, 720.f };

    sf::RenderWindow window;
    LevelData level;
    World world;
    Player player;
    Inventory inventory;
    CameraController camera;
    sf::Clock clock;
    sf::View hudView;
    bool focused{ true };
    bool restartKeyHeld{ false };
    bool inventoryKeyHeld{ false };
    bool debugKeyHeld{ false };
    bool inventoryVisible{ false };
    bool debugDraw{ false };
};
