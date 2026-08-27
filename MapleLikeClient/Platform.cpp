#include "Platform.h"

Platform::Platform(const sf::Vector2f position, const sf::Vector2f size)
    : body(size)
{
    body.setPosition(position);
    body.setFillColor(sf::Color(75, 125, 75));
    body.setOutlineColor(sf::Color(120, 180, 120));
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

void Platform::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    target.draw(body, states);
}
