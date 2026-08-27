#include "Game.h"

#include <algorithm>
#include <optional>

#include "LevelLoader.h"

Game::Game()
    : window(
          sf::VideoMode({ static_cast<unsigned int>(ViewSize.x), static_cast<unsigned int>(ViewSize.y) }),
          "MapleLike RPG"),
      level(LevelLoader::load(LevelLoader::findLevel("assets/maps/level01.json"))),
      world(level),
      player(level.playerSpawn),
      camera(ViewSize, world.getSize()),
      hudView(sf::FloatRect({ 0.f, 0.f }, ViewSize))
{
    window.setFramerateLimit(144);
    camera.handleResize(window.getSize());
    hudView.setViewport(camera.getView().getViewport());
}

void Game::run()
{
    while (window.isOpen())
    {
        processEvents();
        const float deltaTime = std::min(clock.restart().asSeconds(), 1.f / 60.f);

        if (focused)
            update(deltaTime);

        render();
    }
}

void Game::processEvents()
{
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                 keyPressed && keyPressed->code == sf::Keyboard::Key::Escape)
            window.close();
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                 keyPressed && keyPressed->code == sf::Keyboard::Key::R && !restartKeyHeld)
        {
            restartKeyHeld = true;
            restartFromCheckpoint();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                 keyPressed && keyPressed->code == sf::Keyboard::Key::I && !inventoryKeyHeld)
        {
            inventoryKeyHeld = true;
            inventoryVisible = !inventoryVisible;
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                 keyPressed && keyPressed->code == sf::Keyboard::Key::Num1)
        {
            if (inventory.getCount(ItemType::HealthPotion) > 0 && player.heal(2))
                inventory.consume(ItemType::HealthPotion);
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                 keyPressed && keyPressed->code == sf::Keyboard::Key::F3 && !debugKeyHeld)
        {
            debugKeyHeld = true;
            debugDraw = !debugDraw;
            player.setDebugDraw(debugDraw);
        }
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>();
                 keyReleased && keyReleased->code == sf::Keyboard::Key::R)
            restartKeyHeld = false;
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>();
                 keyReleased && keyReleased->code == sf::Keyboard::Key::I)
            inventoryKeyHeld = false;
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>();
                 keyReleased && keyReleased->code == sf::Keyboard::Key::F3)
            debugKeyHeld = false;
        else if (event->is<sf::Event::FocusLost>())
        {
            focused = false;
            restartKeyHeld = false;
            inventoryKeyHeld = false;
            debugKeyHeld = false;
        }
        else if (event->is<sf::Event::FocusGained>())
        {
            focused = true;
            clock.restart();
        }
        else if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            camera.handleResize(resized->size);
            hudView.setViewport(camera.getView().getViewport());
        }
    }
}

void Game::update(const float deltaTime)
{
    player.update(deltaTime, world.getPlatforms(), world.getSize());
    world.update(player, inventory, deltaTime);
    if (player.consumeRespawnRequest())
    {
        restartFromCheckpoint();
    }
    camera.update(player.getPosition() + sf::Vector2f{ 24.f, 36.f }, deltaTime);
}

void Game::restartFromCheckpoint()
{
    player.respawn();
    world.resetEnemies();
}

void Game::render()
{
    window.setView(camera.getView());
    window.clear(sf::Color(30, 35, 45));
    world.draw(window);
    window.draw(player);
    renderHud();
    window.display();
}

void Game::renderHud()
{
    window.setView(hudView);

    constexpr sf::Vector2f barPosition{ 28.f, 28.f };
    constexpr sf::Vector2f barSize{ 260.f, 24.f };
    sf::RectangleShape background(barSize);
    background.setPosition(barPosition);
    background.setFillColor(sf::Color(40, 40, 45, 220));
    background.setOutlineColor(sf::Color::White);
    background.setOutlineThickness(2.f);

    const float healthRatio = static_cast<float>(player.getHealth())
        / static_cast<float>(player.getMaxHealth());
    sf::RectangleShape healthBar({ barSize.x * healthRatio, barSize.y });
    healthBar.setPosition(barPosition);
    healthBar.setFillColor(sf::Color(220, 65, 75));

    window.draw(background);
    window.draw(healthBar);

    if (inventoryVisible)
    {
        renderInventory();
    }
}

void Game::renderInventory()
{
    constexpr sf::Vector2f panelPosition{ 28.f, 72.f };
    sf::RectangleShape panel({ 360.f, 150.f });
    panel.setPosition(panelPosition);
    panel.setFillColor(sf::Color(25, 28, 36, 235));
    panel.setOutlineColor(sf::Color::White);
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    const ItemType types[]{ ItemType::Coin, ItemType::HealthPotion };
    const sf::Color colors[]{ sf::Color(250, 205, 45), sf::Color(225, 65, 150) };
    for (std::size_t slot = 0; slot < 2; ++slot)
    {
        const sf::Vector2f slotPosition{ panelPosition.x + 24.f + static_cast<float>(slot) * 165.f,
                                        panelPosition.y + 24.f };
        sf::RectangleShape slotBody({ 145.f, 100.f });
        slotBody.setPosition(slotPosition);
        slotBody.setFillColor(sf::Color(55, 60, 72));
        slotBody.setOutlineColor(sf::Color(125, 130, 145));
        slotBody.setOutlineThickness(2.f);
        window.draw(slotBody);

        sf::RectangleShape icon({ 34.f, 34.f });
        icon.setPosition(slotPosition + sf::Vector2f{ 12.f, 12.f });
        icon.setFillColor(colors[slot]);
        window.draw(icon);

        const unsigned int count = inventory.getCount(types[slot]);
        for (unsigned int index = 0; index < count; ++index)
        {
            sf::RectangleShape pip({ 8.f, 8.f });
            const float x = slotPosition.x + 12.f + static_cast<float>(index % 10) * 11.f;
            const float y = slotPosition.y + 62.f + static_cast<float>(index / 10) * 12.f;
            pip.setPosition({ x, y });
            pip.setFillColor(colors[slot]);
            window.draw(pip);
        }
    }
}
