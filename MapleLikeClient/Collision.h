#pragma once

#include <SFML/Graphics/Rect.hpp>

namespace Collision
{
    [[nodiscard]] inline bool overlaps(const sf::FloatRect& left, const sf::FloatRect& right)
    {
        return left.position.x < right.position.x + right.size.x
            && left.position.x + left.size.x > right.position.x
            && left.position.y < right.position.y + right.size.y
            && left.position.y + left.size.y > right.position.y;
    }
}
