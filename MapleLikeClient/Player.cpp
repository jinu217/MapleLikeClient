#include "Player.h"

#include <algorithm>

#include "Platform.h"
#include "Collision.h"
#include "Climbable.h"

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
    const std::span<const Climbable> climbables,
    const sf::Vector2f worldSize)
{
    updateDamageTimers(deltaTime);

    bool climbingThisFrame = false;
    if (hurtTimer <= 0.f)
    {
        climbingThisFrame = updateClimbing(climbables);
        if (!climbingThisFrame)
        {
            handleInput(deltaTime);
            updateJump(deltaTime);
        }
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

    if (climbingThisFrame)
    {
        body.move(velocity * deltaTime);

        const float top = activeClimbable->getPosition().y;
        const float bottom = top + activeClimbable->getSize().y;
        const float clampedBottom = std::clamp(
            body.getPosition().y + body.getSize().y,
            top,
            bottom);
        body.setPosition({ body.getPosition().x, clampedBottom - body.getSize().y });
        constrainToWorld(worldSize.x);
        return;
    }

    grounded = false;
    groundedOnOneWay = false;
    applyGravity(deltaTime);

    body.move({ velocity.x * deltaTime, 0.f });
    resolveHorizontalCollisions(platforms);
    constrainToWorld(worldSize.x);

    const float previousBottom = body.getPosition().y + body.getSize().y;
    body.move({ 0.f, velocity.y * deltaTime });
    resolveVerticalCollisions(platforms, previousBottom);

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
    groundedOnOneWay = false;
    climbing = false;
    activeClimbable = nullptr;
    jumpWasPressed = false;
    attackWasPressed = false;
    jumpsUsed = 0;
    coyoteTimer = 0.f;
    jumpBufferTimer = 0.f;
    attackTimer = 0.f;
    attackCooldownTimer = 0.f;
    invulnerabilityTimer = 0.f;
    hurtTimer = 0.f;
    dropThroughTimer = 0.f;
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
    groundedOnOneWay = false;
    climbing = false;
    activeClimbable = nullptr;
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
    const bool downIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

    if (jumpIsPressed && !jumpWasPressed)
    {
        if (downIsPressed && grounded && groundedOnOneWay)
        {
            dropThroughTimer = DropThroughDuration;
            velocity.y = DropThroughSpeed;
            grounded = false;
            groundedOnOneWay = false;
            coyoteTimer = 0.f;
            jumpBufferTimer = 0.f;
            jumpsUsed = 1;
        }
        else
        {
            jumpBufferTimer = JumpBufferDuration;
        }
    }

    if (!jumpIsPressed && jumpWasPressed && velocity.y < 0.f)
    {
        velocity.y *= JumpReleaseMultiplier;
    }

    jumpWasPressed = jumpIsPressed;
}

bool Player::updateClimbing(const std::span<const Climbable> climbables)
{
    const bool jumpIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    const bool upIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
    const bool downIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
    const bool leftIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    const bool rightIsPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    const float verticalDirection = static_cast<float>(downIsPressed) - static_cast<float>(upIsPressed);

    if (!climbing && verticalDirection != 0.f && !jumpIsPressed)
    {
        activeClimbable = findClimbable(climbables);
        if (activeClimbable != nullptr)
        {
            climbing = true;
            grounded = false;
            groundedOnOneWay = false;
            coyoteTimer = 0.f;
            jumpBufferTimer = 0.f;
            jumpsUsed = 0;
        }
    }

    if (!climbing || activeClimbable == nullptr)
    {
        return false;
    }

    if (jumpIsPressed && !jumpWasPressed)
    {
        climbing = false;
        activeClimbable = nullptr;
        velocity.x = facingDirection == FacingDirection::Right
            ? ClimbJumpHorizontalSpeed
            : -ClimbJumpHorizontalSpeed;
        velocity.y = -ClimbJumpSpeed;
        jumpsUsed = 1;
        jumpWasPressed = true;
        return false;
    }

    if (leftIsPressed != rightIsPressed)
    {
        facingDirection = leftIsPressed ? FacingDirection::Left : FacingDirection::Right;
        climbing = false;
        activeClimbable = nullptr;
        velocity.x = leftIsPressed ? -ClimbJumpHorizontalSpeed : ClimbJumpHorizontalSpeed;
        velocity.y = 0.f;
        return false;
    }

    const float top = activeClimbable->getPosition().y;
    const float bottom = top + activeClimbable->getSize().y;
    const float playerBottom = body.getPosition().y + body.getSize().y;
    if ((verticalDirection < 0.f && playerBottom <= top + 1.f)
        || (verticalDirection > 0.f && playerBottom >= bottom - 1.f))
    {
        climbing = false;
        activeClimbable = nullptr;
        velocity = {};
        return false;
    }

    const float climbCenterX = activeClimbable->getPosition().x
        + activeClimbable->getSize().x / 2.f;
    body.setPosition({ climbCenterX - body.getSize().x / 2.f, body.getPosition().y });
    velocity = { 0.f, verticalDirection * ClimbSpeed };
    jumpWasPressed = jumpIsPressed;
    return true;
}

const Climbable* Player::findClimbable(const std::span<const Climbable> climbables) const
{
    const sf::FloatRect playerBounds = getBounds();
    const float playerCenterX = playerBounds.position.x + playerBounds.size.x / 2.f;
    const float playerBottom = playerBounds.position.y + playerBounds.size.y;

    for (const Climbable& climbable : climbables)
    {
        const sf::Vector2f position = climbable.getPosition();
        const sf::Vector2f size = climbable.getSize();
        const bool horizontallyAligned = playerCenterX >= position.x - 12.f
            && playerCenterX <= position.x + size.x + 12.f;
        const bool verticallyAligned = playerBottom >= position.y - 4.f
            && playerBounds.position.y <= position.y + size.y + 4.f;
        if (horizontallyAligned && verticallyAligned)
        {
            return &climbable;
        }
    }

    return nullptr;
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
    dropThroughTimer = std::max(0.f, dropThroughTimer - deltaTime);
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
        if (platform.getType() == PlatformType::OneWay)
        {
            continue;
        }

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

void Player::resolveVerticalCollisions(
    const std::span<const Platform> platforms,
    const float previousBottom)
{
    for (const Platform& platform : platforms)
    {
        if (!overlaps(platform))
        {
            continue;
        }

        const sf::Vector2f platformPosition = platform.getPosition();
        const sf::Vector2f platformSize = platform.getSize();

        if (platform.getType() == PlatformType::OneWay)
        {
            const bool landedFromAbove = velocity.y >= 0.f
                && previousBottom <= platformPosition.y + 1.f;
            if (dropThroughTimer > 0.f || !landedFromAbove)
            {
                continue;
            }
        }

        if (velocity.y > 0.f)
        {
            body.setPosition({ body.getPosition().x, platformPosition.y - body.getSize().y });
            grounded = true;
            groundedOnOneWay = platform.getType() == PlatformType::OneWay;
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
