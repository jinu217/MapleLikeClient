#pragma once

#include <SFML/System/Vector2.hpp>

#include <vector>

#include "Platform.h"
#include "Climbable.h"

struct PlatformData
{
    sf::Vector2f position;
    sf::Vector2f size;
    PlatformType type;
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

struct ClimbableData
{
    sf::Vector2f position;
    sf::Vector2f size;
    ClimbableType type;
};

struct LevelData
{
    sf::Vector2f worldSize;
    sf::Vector2f playerSpawn;
    std::vector<PlatformData> platforms;
    std::vector<EnemyData> enemies;
    std::vector<CheckpointData> checkpoints;
    std::vector<ClimbableData> climbables;
};
