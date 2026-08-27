#include "CameraController.h"

#include <algorithm>
#include <cmath>

CameraController::CameraController(const sf::Vector2f viewSize, const sf::Vector2f worldSize)
    : view(sf::FloatRect({ 0.f, 0.f }, viewSize)), viewSize(viewSize), worldSize(worldSize)
{
    view.setCenter(viewSize / 2.f);
}

void CameraController::update(const sf::Vector2f targetPosition, const float deltaTime)
{
    const sf::Vector2f halfView = viewSize / 2.f;
    const sf::Vector2f targetCenter{
        std::clamp(targetPosition.x, halfView.x, worldSize.x - halfView.x),
        std::clamp(targetPosition.y, halfView.y, worldSize.y - halfView.y)
    };
    const float followAmount = 1.f - std::exp(-FollowSpeed * deltaTime);
    view.setCenter(view.getCenter() + (targetCenter - view.getCenter()) * followAmount);
}

void CameraController::handleResize(const sf::Vector2u windowSize)
{
    if (windowSize.x == 0 || windowSize.y == 0)
        return;

    const float windowAspect = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float viewAspect = viewSize.x / viewSize.y;
    sf::FloatRect viewport({ 0.f, 0.f }, { 1.f, 1.f });

    if (windowAspect > viewAspect)
    {
        viewport.size.x = viewAspect / windowAspect;
        viewport.position.x = (1.f - viewport.size.x) / 2.f;
    }
    else
    {
        viewport.size.y = windowAspect / viewAspect;
        viewport.position.y = (1.f - viewport.size.y) / 2.f;
    }

    view.setViewport(viewport);
}

const sf::View& CameraController::getView() const
{
    return view;
}
