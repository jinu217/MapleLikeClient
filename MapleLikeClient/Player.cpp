#include "Player.h"

#include <algorithm>

#include "Platform.h"
#include "Collision.h"

Player::Player(const sf::Vector2f startPosition)
    : body({ 48.f, 72.f }), spawnPosition(startPosition)
{
    body.setPosition(startPosition);
    body.setFillColor(sf::Color(80, 170, 255));
    body.setOutlineColor(sf::Color::White);
    body.setOutlineThickness(2.f);
}

void Player::update(
    const float deltaTime,
    const std::span<const Platform> platforms,
    const sf::Vector2f worldSize)
{
    updateDamageTimers(deltaTime);

    if (hurtTimer <= 0.f)
    {
        handleInput(deltaTime);
        updateJump(deltaTime);
        updateAttack(deltaTime);
    }
    else
    {
        jumpWasPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
        attackWasPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
        jumpBufferTimer = 0.f;
        attackTimer = std::max(0.f, attackTimer - deltaTime);
        attackCooldownTimer = std::max(0.f, attackCooldownTimer - deltaTime);
    }
    grounded = false;
    applyGravity(deltaTime);

    body.move({ velocity.x * deltaTime, 0.f });
    resolveHorizontalCollisions(platforms);
    constrainToWorld(worldSize.x);

    body.move({ 0.f, velocity.y * deltaTime });
    resolveVerticalCollisions(platforms);

    if (grounded && jumpBufferTimer > 0.f)
    {
        startJump();
    }

    if (body.getPosition().y > worldSize.y)
    {
        respawnRequested = true;
    }
}

void Player::respawn()
{
    body.setPosition(spawnPosition);
    velocity = {};
    grounded = false;
    jumpWasPressed = false;
    attackWasPressed = false;
    jumpsUsed = 0;
    coyoteTimer = 0.f;
    jumpBufferTimer = 0.f;
    attackTimer = 0.f;
    attackCooldownTimer = 0.f;
    invulnerabilityTimer = 0.f;
    hurtTimer = 0.f;
    health = MaxHealth;
}

void Player::setSpawnPosition(const sf::Vector2f position)
{
    spawnPosition = position;
}

void Player::setDebugDraw(const bool enabled)
{
    debugDraw = enabled;
}

bool Player::consumeRespawnRequest()
{
    const bool requested = respawnRequested;
    respawnRequested = false;
    return requested;
}

sf::Vector2f Player::getPosition() const
{
    return body.getPosition();
}

sf::Vector2f Player::getVelocity() const
{
    return velocity;
}

bool Player::isGrounded() const
{
    return grounded;
}

Player::FacingDirection Player::getFacingDirection() const
{
    return facingDirection;
}

bool Player::isAttacking() const
{
    return attackTimer > 0.f;
}

std::optional<sf::FloatRect> Player::getAttackBounds() const
{
    if (!isAttacking())
    {
        return std::nullopt;
    }

    const sf::Vector2f position = body.getPosition();
    const float attackX = facingDirection == FacingDirection::Right
        ? position.x + body.getSize().x
        : position.x - AttackSize.x;

    return sf::FloatRect({ attackX, position.y + 7.f }, AttackSize);
}

std::uint64_t Player::getAttackId() const
{
    return attackId;
}

sf::FloatRect Player::getBounds() const
{
    return sf::FloatRect(body.getPosition(), body.getSize());
}

int Player::getHealth() const
{
    return health;
}

int Player::getMaxHealth() const
{
    return MaxHealth;
}

bool Player::takeDamage(const sf::Vector2f damageSource)
{
    if (invulnerabilityTimer > 0.f)
    {
        return false;
    }

    --health;
    if (health <= 0)
    {
        respawnRequested = true;
        return true;
    }

    const float playerCenterX = body.getPosition().x + body.getSize().x / 2.f;
    velocity.x = playerCenterX < damageSource.x ? -HurtKnockbackSpeed : HurtKnockbackSpeed;
    velocity.y = -HurtKnockbackLift;
    grounded = false;
    hurtTimer = HurtDuration;
    invulnerabilityTimer = InvulnerabilityDuration;
    jumpBufferTimer = 0.f;
    attackTimer = 0.f;
    return true;
}

bool Player::heal(const int amount)
{
    if (health >= MaxHealth || amount <= 0)
    {
        return false;
    }

    health = std::min(health + amount, MaxHealth);
    return true;
}

void Player::handleInput(const float deltaTime)
{
    float direction = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        direction -= 1.f;
        facingDirection = FacingDirection::Left;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        direction += 1.f;
        facingDirection = FacingDirection::Right;
    }

    const float targetSpeed = direction * MoveSpeed;
    const float acceleration = grounded
        ? (direction == 0.f ? GroundDeceleration : GroundAcceleration)
        : AirAcceleration;
    const float speedDifference = targetSpeed - velocity.x;
    const float speedChange = acceleration * deltaTime;
    velocity.x += std::clamp(speedDifference, -speedChange, speedChange);

    const bool jumpIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    if (jumpIsPressed && !jumpWasPressed)
    {
        jumpBufferTimer = JumpBufferDuration;
    }

    if (!jumpIsPressed && jumpWasPressed && velocity.y < 0.f)
    {
        velocity.y *= JumpReleaseMultiplier;
    }

    jumpWasPressed = jumpIsPressed;
}

void Player::updateJump(const float deltaTime)
{
    jumpBufferTimer = std::max(0.f, jumpBufferTimer - deltaTime);

    if (grounded)
    {
        coyoteTimer = CoyoteDuration;
    }
    else
    {
        coyoteTimer = std::max(0.f, coyoteTimer - deltaTime);

        if (coyoteTimer == 0.f && jumpsUsed == 0)
        {
            jumpsUsed = 1;
        }
    }

    if (jumpBufferTimer <= 0.f)
    {
        return;
    }

    if (grounded || coyoteTimer > 0.f || jumpsUsed < MaxJumps)
    {
        startJump();
    }
}

void Player::startJump()
{
    velocity.y = -JumpSpeed;
    grounded = false;
    coyoteTimer = 0.f;
    jumpBufferTimer = 0.f;
    ++jumpsUsed;
}

void Player::updateAttack(const float deltaTime)
{
    attackTimer = std::max(0.f, attackTimer - deltaTime);
    attackCooldownTimer = std::max(0.f, attackCooldownTimer - deltaTime);

    const bool attackIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::J)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);

    if (attackIsPressed && !attackWasPressed && attackCooldownTimer <= 0.f)
    {
        attackTimer = AttackDuration;
        attackCooldownTimer = AttackCooldown;
        ++attackId;
    }

    attackWasPressed = attackIsPressed;
}

void Player::updateDamageTimers(const float deltaTime)
{
    invulnerabilityTimer = std::max(0.f, invulnerabilityTimer - deltaTime);
    hurtTimer = std::max(0.f, hurtTimer - deltaTime);
}

void Player::applyGravity(const float deltaTime)
{
    if (!grounded)
    {
        velocity.y = std::min(velocity.y + Gravity * deltaTime, MaxFallSpeed);
    }
}

void Player::resolveHorizontalCollisions(const std::span<const Platform> platforms)
{
    for (const Platform& platform : platforms)
    {
        if (!overlaps(platform))
        {
            continue;
        }

        const sf::Vector2f platformPosition = platform.getPosition();
        const sf::Vector2f platformSize = platform.getSize();

        if (velocity.x > 0.f)
        {
            body.setPosition({ platformPosition.x - body.getSize().x, body.getPosition().y });
        }
        else if (velocity.x < 0.f)
        {
            body.setPosition({ platformPosition.x + platformSize.x, body.getPosition().y });
        }

        velocity.x = 0.f;
    }
}

void Player::resolveVerticalCollisions(const std::span<const Platform> platforms)
{
    for (const Platform& platform : platforms)
    {
        if (!overlaps(platform))
        {
            continue;
        }

        const sf::Vector2f platformPosition = platform.getPosition();
        const sf::Vector2f platformSize = platform.getSize();

        if (velocity.y > 0.f)
        {
            body.setPosition({ body.getPosition().x, platformPosition.y - body.getSize().y });
            grounded = true;
            jumpsUsed = 0;
        }
        else if (velocity.y < 0.f)
        {
            body.setPosition({ body.getPosition().x, platformPosition.y + platformSize.y });
        }

        velocity.y = 0.f;
    }
}

void Player::constrainToWorld(const float worldWidth)
{
    const float maxX = std::max(0.f, worldWidth - body.getSize().x);
    const float clampedX = std::clamp(body.getPosition().x, 0.f, maxX);

    if (clampedX != body.getPosition().x)
    {
        body.setPosition({ clampedX, body.getPosition().y });
        velocity.x = 0.f;
    }
}

bool Player::overlaps(const Platform& platform) const
{
    return Collision::overlaps(
        getBounds(),
        sf::FloatRect(platform.getPosition(), platform.getSize()));
}

void Player::draw(sf::RenderTarget& target, const sf::RenderStates states) const
{
    const bool visible = invulnerabilityTimer <= 0.f
        || static_cast<int>(invulnerabilityTimer * 16.f) % 2 == 0;
    if (visible)
    {
        target.draw(body, states);
    }

    if (debugDraw)
    {
        if (const std::optional attackBounds = getAttackBounds())
        {
            sf::RectangleShape debugHitbox(attackBounds->size);
            debugHitbox.setPosition(attackBounds->position);
            debugHitbox.setFillColor(sf::Color(255, 220, 70, 90));
            debugHitbox.setOutlineColor(sf::Color(255, 230, 100));
            debugHitbox.setOutlineThickness(1.f);
            target.draw(debugHitbox, states);
        }
    }
}
