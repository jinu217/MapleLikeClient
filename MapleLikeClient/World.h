#pragma once

#include <SFML/Graphics.hpp>

#include <span>
#include <vector>

#include "Platform.h"
#include "Enemy.h"
#include "Checkpoint.h"
#include "LevelData.h"
#include "ItemDrop.h"
#include "Climbable.h"

class Player;
class Inventory;

class World
{
public:
    explicit World(const LevelData& level);

    [[nodiscard]] sf::Vector2f getSize() const;
    [[nodiscard]] std::span<const Platform> getPlatforms() const;
    [[nodiscard]] std::span<const Climbable> getClimbables() const;
    void update(Player& player, Inventory& inventory, float deltaTime);
    void resetEnemies();
    void draw(sf::RenderTarget& target) const;

private:
    sf::Vector2f size;
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    std::vector<EnemyData> enemySpawns;
    std::vector<Checkpoint> checkpoints;
    std::vector<Climbable> climbables;
    std::vector<ItemDrop> itemDrops;
    std::size_t activeCheckpoint{ static_cast<std::size_t>(-1) };
};
