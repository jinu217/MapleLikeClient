#pragma once

#include <SFML/Graphics.hpp>

enum class ClimbableType
{
    Ladder,
    Rope
};

class Climbable final : public sf::Drawable
{
public:
    Climbable(sf::Vector2f position, sf::Vector2f size, ClimbableType type);

    [[nodiscard]] sf::Vector2f getPosition() const;
    [[nodiscard]] sf::Vector2f getSize() const;
    [[nodiscard]] ClimbableType getType() const;

private:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
    ClimbableType type;
};
