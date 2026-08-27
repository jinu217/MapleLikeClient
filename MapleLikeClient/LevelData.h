#pragma once

#include <SFML/System/Vector2.hpp>

#include <vector>

struct PlatformData
{
    sf::Vector2f position;
    sf::Vector2f size;
};

struct EnemyData
{
    sf::Vector2f position;
};

struct CheckpointData
{
    sf::Vector2f position;
    sf::Vector2f size;
};

struct LevelData
{
    sf::Vector2f worldSize;
    sf::Vector2f playerSpawn;
    std::vector<PlatformData> platforms;
    std::vector<EnemyData> enemies;
    std::vector<CheckpointData> checkpoints;
};
