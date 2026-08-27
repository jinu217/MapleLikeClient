#pragma once

#include <SFML/Graphics.hpp>

#include <span>

#include "ItemType.h"

class Platform;

class ItemDrop final : public sf::Drawable
{
public:
    ItemDrop(ItemType type, sf::Vector2f position, float horizontalImpulse);

    void update(float deltaTime, std::span<const Platform> platforms);
    [[nodiscard]] ItemType getType() const;
    [[nodiscard]] sf::FloatRect getBounds() const;
    [[nodiscard]] bool isExpired() const;

private:
    [[nodiscard]] bool overlaps(const Platform& platform) const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
    sf::Vector2f velocity;
    ItemType type;
    float lifetime{ 15.f };

    static constexpr float Gravity = 1'400.f;
    static constexpr float MaxFallSpeed = 700.f;
};
