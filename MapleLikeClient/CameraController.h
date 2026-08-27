#pragma once

#include <SFML/Graphics.hpp>

class CameraController
{
public:
    CameraController(sf::Vector2f viewSize, sf::Vector2f worldSize);

    void update(sf::Vector2f targetPosition, float deltaTime);
    void handleResize(sf::Vector2u windowSize);
    [[nodiscard]] const sf::View& getView() const;

private:
    sf::View view;
    sf::Vector2f viewSize;
    sf::Vector2f worldSize;

    static constexpr float FollowSpeed = 7.f;
};
