#pragma once

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <span>

class Platform;

class Enemy final : public sf::Drawable
{
public:
    explicit Enemy(sf::Vector2f position);

    void update(float deltaTime, std::span<const Platform> platforms);
    bool tryTakeHit(
        const sf::FloatRect& attackBounds,
        std::uint64_t attackId,
        bool attackFacingRight);

    [[nodiscard]] bool isDead() const;
    [[nodiscard]] sf::FloatRect getBounds() const;
    [[nodiscard]] sf::Vector2f getCenter() const;

private:
    [[nodiscard]] bool overlaps(const sf::FloatRect& attackBounds) const;
    [[nodiscard]] bool hasGroundAhead(std::span<const Platform> platforms) const;
    void resolveHorizontalCollisions(std::span<const Platform> platforms);
    void resolveVerticalCollisions(std::span<const Platform> platforms);
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
    sf::Vector2f velocity{};
    float moveDirection{ 1.f };
    bool grounded{ false };
    float knockbackTimer{ 0.f };
    int health{ 3 };
    std::uint64_t lastHitAttackId{ 0 };
    float hitFlashTimer{ 0.f };

    static constexpr float HitFlashDuration = 0.1f;
    static constexpr float MoveSpeed = 80.f;
    static constexpr float Gravity = 1'800.f;
    static constexpr float MaxFallSpeed = 900.f;
    static constexpr float KnockbackSpeed = 320.f;
    static constexpr float KnockbackLift = 260.f;
    static constexpr float KnockbackDuration = 0.18f;
};
