#pragma once

#include <SFML/Graphics.hpp>

class Checkpoint final : public sf::Drawable
{
public:
    Checkpoint(sf::Vector2f position, sf::Vector2f size);

    void setActive(bool active);
    [[nodiscard]] bool contains(const sf::FloatRect& bounds) const;
    [[nodiscard]] sf::Vector2f getSpawnPosition() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
    bool active{ false };
};
