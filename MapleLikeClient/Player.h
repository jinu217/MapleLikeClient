#pragma once
#include <SFML/Graphics.hpp>

#include <span>
#include <cstdint>
#include <optional>

class Platform;

class Player final : public sf::Drawable
{
public:
    enum class FacingDirection
    {
        Left,
        Right
    };

    explicit Player(sf::Vector2f startPosition);

    void update(float deltaTime, std::span<const Platform> platforms, sf::Vector2f worldSize);
    void respawn();
    void setSpawnPosition(sf::Vector2f position);
    void setDebugDraw(bool enabled);
    [[nodiscard]] bool consumeRespawnRequest();

    [[nodiscard]] sf::Vector2f getPosition() const;
    [[nodiscard]] sf::Vector2f getVelocity() const;
    [[nodiscard]] bool isGrounded() const;
    [[nodiscard]] FacingDirection getFacingDirection() const;
    [[nodiscard]] bool isAttacking() const;
    [[nodiscard]] std::optional<sf::FloatRect> getAttackBounds() const;
    [[nodiscard]] std::uint64_t getAttackId() const;
    [[nodiscard]] sf::FloatRect getBounds() const;
    [[nodiscard]] int getHealth() const;
    [[nodiscard]] int getMaxHealth() const;
    bool takeDamage(sf::Vector2f damageSource);
    bool heal(int amount);

private:
    void handleInput(float deltaTime);
    void updateJump(float deltaTime);
    void startJump();
    void updateAttack(float deltaTime);
    void updateDamageTimers(float deltaTime);
    void applyGravity(float deltaTime);
    void resolveHorizontalCollisions(std::span<const Platform> platforms);
    void resolveVerticalCollisions(std::span<const Platform> platforms);
    void constrainToWorld(float worldWidth);
    [[nodiscard]] bool overlaps(const Platform& platform) const;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    sf::RectangleShape body;
    sf::Vector2f spawnPosition;
    sf::Vector2f velocity{};
    bool grounded{ false };
    bool jumpWasPressed{ false };
    bool attackWasPressed{ false };
    unsigned int jumpsUsed{ 0 };
    std::uint64_t attackId{ 0 };
    FacingDirection facingDirection{ FacingDirection::Right };
    float coyoteTimer{ 0.f };
    float jumpBufferTimer{ 0.f };
    float attackTimer{ 0.f };
    float attackCooldownTimer{ 0.f };
    float invulnerabilityTimer{ 0.f };
    float hurtTimer{ 0.f };
    int health{ 5 };
    bool respawnRequested{ false };
    bool debugDraw{ false };

    static constexpr float MoveSpeed = 300.f;
    static constexpr float GroundAcceleration = 2'400.f;
    static constexpr float GroundDeceleration = 3'000.f;
    static constexpr float AirAcceleration = 1'400.f;
    static constexpr float JumpSpeed = 650.f;
    static constexpr float JumpReleaseMultiplier = 0.45f;
    static constexpr float Gravity = 1'800.f;
    static constexpr float MaxFallSpeed = 900.f;
    static constexpr float CoyoteDuration = 0.1f;
    static constexpr float JumpBufferDuration = 0.12f;
    static constexpr float AttackDuration = 0.18f;
    static constexpr float AttackCooldown = 0.35f;
    static constexpr sf::Vector2f AttackSize{ 76.f, 58.f };
    static constexpr int MaxHealth = 5;
    static constexpr float InvulnerabilityDuration = 1.f;
    static constexpr float HurtDuration = 0.22f;
    static constexpr float HurtKnockbackSpeed = 420.f;
    static constexpr float HurtKnockbackLift = 420.f;
    static constexpr unsigned int MaxJumps = 2;
};
