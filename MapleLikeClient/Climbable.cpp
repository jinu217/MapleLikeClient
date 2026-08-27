#include "Climbable.h"

Climbable::Climbable(
    const sf::Vector2f position,
    const sf::Vector2f size,
    const ClimbableType type)
    : body(size), type(type)
{
    body.setPosition(position);
    body.setFillColor(type == ClimbableType::Ladder
        ? sf::Color(175, 125, 70, 180)
        : sf::Color(210, 190, 115, 210));
    body.setOutlineColor(sf::Color(235, 220, 170));
    body.setOutlineThickness(1.f);
}

sf::Vector2f Climbable::getPosition() const
{
    return body.getPosition();
}

sf::Vector2f Climbable::getSize() const
{
    return body.getSize();
}

ClimbableType Climbable::getType() const
{
    return type;
}

void Climbable::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    target.draw(body, states);
}
