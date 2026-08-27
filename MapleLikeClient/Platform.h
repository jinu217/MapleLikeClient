#pragma once

#include <SFML/Graphics.hpp>

class Platform final : public sf::Drawable
{
public:
    Platform(sf::Vector2f position, sf::Vector2f size);

    [[nodiscard]] sf::Vector2f getPosition() const;
    [[nodiscard]] sf::Vector2f getSize() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
};
