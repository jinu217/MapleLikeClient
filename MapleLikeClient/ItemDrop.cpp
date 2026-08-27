#include "ItemDrop.h"

#include <algorithm>

#include "Platform.h"
#include "Collision.h"

ItemDrop::ItemDrop(const ItemType type, const sf::Vector2f position, const float horizontalImpulse)
    : body({ 24.f, 24.f }), velocity({ horizontalImpulse, -300.f }), type(type)
{
    body.setPosition(position - body.getSize() / 2.f);
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);
}

void ItemDrop::update(const float deltaTime, const std::span<const Platform> platforms)
{
    lifetime -= deltaTime;
    velocity.x *= std::max(0.f, 1.f - 2.5f * deltaTime);
    velocity.y = std::min(velocity.y + Gravity * deltaTime, MaxFallSpeed);
    const float previousBottom = body.getPosition().y + body.getSize().y;
    body.move(velocity * deltaTime);

    if (velocity.y < 0.f)
    {
        return;
    }

    for (const Platform& platform : platforms)
    {
        if (!overlaps(platform))
        {
            continue;
        }

        if (platform.getType() == PlatformType::OneWay
            && previousBottom > platform.getPosition().y + 1.f)
        {
            continue;
        }

        body.setPosition({ body.getPosition().x, platform.getPosition().y - body.getSize().y });
        velocity.y = 0.f;
        break;
    }
}

ItemType ItemDrop::getType() const
{
    return type;
}

sf::FloatRect ItemDrop::getBounds() const
{
    return sf::FloatRect(body.getPosition(), body.getSize());
}

bool ItemDrop::isExpired() const
{
    return lifetime <= 0.f;
}

bool ItemDrop::overlaps(const Platform& platform) const
{
    return Collision::overlaps(
        getBounds(),
        sf::FloatRect(platform.getPosition(), platform.getSize()));
}

void ItemDrop::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    sf::RectangleShape renderedBody = body;
    renderedBody.setFillColor(type == ItemType::Coin
        ? sf::Color(250, 205, 45)
        : sf::Color(225, 65, 150));
    target.draw(renderedBody, states);
}
