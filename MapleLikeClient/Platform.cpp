#include "Platform.h"

Platform::Platform(const sf::Vector2f position, const sf::Vector2f size, const PlatformType type)
    : body(size), type(type)
{
    body.setPosition(position);
    body.setFillColor(type == PlatformType::Solid
        ? sf::Color(75, 125, 75)
        : sf::Color(90, 135, 180));
    body.setOutlineColor(type == PlatformType::Solid
        ? sf::Color(120, 180, 120)
        : sf::Color(145, 195, 235));
    body.setOutlineThickness(2.f);
}

sf::Vector2f Platform::getPosition() const
{
    return body.getPosition();
}

sf::Vector2f Platform::getSize() const
{
    return body.getSize();
}

PlatformType Platform::getType() const
{
    return type;
}

void Platform::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    target.draw(body, states);
}
