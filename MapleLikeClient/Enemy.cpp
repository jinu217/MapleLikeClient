#include "Enemy.h"

#include <algorithm>

#include "Platform.h"
#include "Collision.h"

Enemy::Enemy(const sf::Vector2f position)
    : body({ 56.f, 64.f })
{
    body.setPosition(position);
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);
}

void Enemy::update(const float deltaTime, const std::span<const Platform> platforms)
{
    hitFlashTimer = std::max(0.f, hitFlashTimer - deltaTime);
    knockbackTimer = std::max(0.f, knockbackTimer - deltaTime);

    if (knockbackTimer <= 0.f && grounded && !hasGroundAhead(platforms))
    {
        moveDirection *= -1.f;
    }

    if (knockbackTimer <= 0.f)
    {
        velocity.x = moveDirection * MoveSpeed;
    }
    velocity.y = std::min(velocity.y + Gravity * deltaTime, MaxFallSpeed);

    body.move({ velocity.x * deltaTime, 0.f });
    resolveHorizontalCollisions(platforms);

    const float previousBottom = body.getPosition().y + body.getSize().y;
    grounded = false;
    body.move({ 0.f, velocity.y * deltaTime });
    resolveVerticalCollisions(platforms, previousBottom);
}

bool Enemy::tryTakeHit(
    const sf::FloatRect& attackBounds,
    const std::uint64_t attackId,
    const bool attackFacingRight)
{
    if (isDead() || attackId == lastHitAttackId || !overlaps(attackBounds))
    {
        return false;
    }

    lastHitAttackId = attackId;
    --health;
    hitFlashTimer = HitFlashDuration;
    velocity.x = attackFacingRight ? KnockbackSpeed : -KnockbackSpeed;
    velocity.y = -KnockbackLift;
    moveDirection = attackFacingRight ? 1.f : -1.f;
    grounded = false;
    knockbackTimer = KnockbackDuration;
    return true;
}

bool Enemy::isDead() const
{
    return health <= 0;
}

sf::FloatRect Enemy::getBounds() const
{
    return sf::FloatRect(body.getPosition(), body.getSize());
}

sf::Vector2f Enemy::getCenter() const
{
    return body.getPosition() + body.getSize() / 2.f;
}

bool Enemy::overlaps(const sf::FloatRect& attackBounds) const
{
    return Collision::overlaps(getBounds(), attackBounds);
}

bool Enemy::hasGroundAhead(const std::span<const Platform> platforms) const
{
    const float probeX = moveDirection > 0.f
        ? body.getPosition().x + body.getSize().x + 2.f
        : body.getPosition().x - 2.f;
    const float probeY = body.getPosition().y + body.getSize().y + 4.f;

    for (const Platform& platform : platforms)
    {
        const sf::Vector2f position = platform.getPosition();
        const sf::Vector2f size = platform.getSize();
        if (probeX >= position.x && probeX <= position.x + size.x
            && probeY >= position.y && probeY <= position.y + 8.f)
        {
            return true;
        }
    }

    return false;
}

void Enemy::resolveHorizontalCollisions(const std::span<const Platform> platforms)
{
    for (const Platform& platform : platforms)
    {
        if (platform.getType() == PlatformType::OneWay)
            continue;

        if (!overlaps(sf::FloatRect(platform.getPosition(), platform.getSize())))
            continue;

        if (velocity.x > 0.f)
            body.setPosition({ platform.getPosition().x - body.getSize().x, body.getPosition().y });
        else if (velocity.x < 0.f)
            body.setPosition({ platform.getPosition().x + platform.getSize().x, body.getPosition().y });

        moveDirection *= -1.f;
        velocity.x = 0.f;
    }
}

void Enemy::resolveVerticalCollisions(
    const std::span<const Platform> platforms,
    const float previousBottom)
{
    for (const Platform& platform : platforms)
    {
        if (!overlaps(sf::FloatRect(platform.getPosition(), platform.getSize())))
            continue;

        if (platform.getType() == PlatformType::OneWay
            && (velocity.y < 0.f || previousBottom > platform.getPosition().y + 1.f))
            continue;

        if (velocity.y > 0.f)
        {
            body.setPosition({ body.getPosition().x, platform.getPosition().y - body.getSize().y });
            grounded = true;
        }
        else if (velocity.y < 0.f)
        {
            body.setPosition({ body.getPosition().x, platform.getPosition().y + platform.getSize().y });
        }

        velocity.y = 0.f;
    }
}

void Enemy::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    sf::RectangleShape renderedBody = body;
    renderedBody.setFillColor(hitFlashTimer > 0.f ? sf::Color::White : sf::Color(210, 70, 70));
    target.draw(renderedBody, states);
}
