#include "Checkpoint.h"

#include "Collision.h"

Checkpoint::Checkpoint(const sf::Vector2f position, const sf::Vector2f size)
    : body(size)
{
    body.setPosition(position);
    body.setOutlineThickness(2.f);
}

void Checkpoint::setActive(const bool isActive)
{
    active = isActive;
}

bool Checkpoint::contains(const sf::FloatRect& bounds) const
{
    return Collision::overlaps(sf::FloatRect(body.getPosition(), body.getSize()), bounds);
}

sf::Vector2f Checkpoint::getSpawnPosition() const
{
    return body.getPosition();
}

void Checkpoint::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    sf::RectangleShape renderedBody = body;
    renderedBody.setFillColor(active ? sf::Color(80, 220, 255, 180) : sf::Color(110, 110, 130, 150));
    renderedBody.setOutlineColor(active ? sf::Color::White : sf::Color(170, 170, 185));
    target.draw(renderedBody, states);
}
