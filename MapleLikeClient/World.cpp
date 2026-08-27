#include "World.h"

#include <algorithm>

#include "Player.h"
#include "Inventory.h"
#include "Collision.h"

World::World(const LevelData& level)
    : size(level.worldSize), enemySpawns(level.enemies)
{
    platforms.reserve(level.platforms.size());
    for (const PlatformData& platform : level.platforms)
    {
        platforms.emplace_back(platform.position, platform.size, platform.type);
    }

    resetEnemies();

    checkpoints.reserve(level.checkpoints.size());
    for (const CheckpointData& checkpoint : level.checkpoints)
    {
        checkpoints.emplace_back(checkpoint.position, checkpoint.size);
    }

    climbables.reserve(level.climbables.size());
    for (const ClimbableData& climbable : level.climbables)
    {
        climbables.emplace_back(climbable.position, climbable.size, climbable.type);
    }
}

sf::Vector2f World::getSize() const
{
    return size;
}

std::span<const Platform> World::getPlatforms() const
{
    return platforms;
}

std::span<const Climbable> World::getClimbables() const
{
    return climbables;
}

void World::update(Player& player, Inventory& inventory, const float deltaTime)
{
    const sf::FloatRect initialPlayerBounds = player.getBounds();
    for (std::size_t index = 0; index < checkpoints.size(); ++index)
    {
        if (!checkpoints[index].contains(initialPlayerBounds) || activeCheckpoint == index)
        {
            continue;
        }

        activeCheckpoint = index;
        for (std::size_t other = 0; other < checkpoints.size(); ++other)
        {
            checkpoints[other].setActive(other == activeCheckpoint);
        }
        player.setSpawnPosition(checkpoints[index].getSpawnPosition());
        break;
    }

    for (Enemy& enemy : enemies)
    {
        enemy.update(deltaTime, platforms);
    }

    if (const std::optional attackBounds = player.getAttackBounds())
    {
        for (Enemy& enemy : enemies)
        {
            const bool hit = enemy.tryTakeHit(
                *attackBounds,
                player.getAttackId(),
                player.getFacingDirection() == Player::FacingDirection::Right);
            if (hit && enemy.isDead())
            {
                const ItemType dropType = static_cast<int>(enemy.getCenter().x / 100.f) % 2 == 1
                    ? ItemType::HealthPotion
                    : ItemType::Coin;
                const float impulse = player.getFacingDirection() == Player::FacingDirection::Right
                    ? 150.f
                    : -150.f;
                itemDrops.emplace_back(dropType, enemy.getCenter(), impulse);
            }
        }
    }

    for (ItemDrop& item : itemDrops)
    {
        item.update(deltaTime, platforms);
    }

    const sf::FloatRect pickupBounds = player.getBounds();
    std::erase_if(itemDrops, [&](const ItemDrop& item)
    {
        const sf::FloatRect bounds = item.getBounds();
        const bool touching = Collision::overlaps(pickupBounds, bounds);
        return item.isExpired() || (touching && inventory.add(item.getType()));
    });

    const sf::FloatRect playerBounds = player.getBounds();
    for (const Enemy& enemy : enemies)
    {
        if (enemy.isDead())
        {
            continue;
        }

        const sf::FloatRect enemyBounds = enemy.getBounds();
        const bool touching = Collision::overlaps(playerBounds, enemyBounds);

        if (touching)
        {
            player.takeDamage(enemy.getCenter());
            break;
        }
    }

    std::erase_if(enemies, [](const Enemy& enemy) { return enemy.isDead(); });
}

void World::resetEnemies()
{
    enemies.clear();
    itemDrops.clear();
    enemies.reserve(enemySpawns.size());
    for (const EnemyData& enemy : enemySpawns)
    {
        enemies.emplace_back(enemy.position);
    }
}

void World::draw(sf::RenderTarget& target) const
{
    for (const Climbable& climbable : climbables)
    {
        target.draw(climbable);
    }

    for (const Platform& platform : platforms)
    {
        target.draw(platform);
    }

    for (const Enemy& enemy : enemies)
    {
        target.draw(enemy);
    }

    for (const Checkpoint& checkpoint : checkpoints)
    {
        target.draw(checkpoint);
    }

    for (const ItemDrop& item : itemDrops)
    {
        target.draw(item);
    }
}
