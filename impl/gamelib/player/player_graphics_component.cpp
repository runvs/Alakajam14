#include "player_graphics_component.hpp"
#include "game_properties.hpp"
#include "player.hpp"

PlayerGraphicsComponent::PlayerGraphicsComponent(std::shared_ptr<jt::GameInterface> gameInterface)
{
    createAnimation(gameInterface->gfx().textureManager());
}

void PlayerGraphicsComponent::createAnimation(jt::TextureManagerInterface& textureManager)
{
    auto const frameTimeAttack = 0.05f;

    m_animation = std::make_shared<jt::Animation>();

    // General
    {
        m_animation->add("assets/player.png", "idle",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 30, 30, 31, 32, 33, 34, 35, 36 }, 0.25f, textureManager);

        m_animation->add("assets/player.png", "attack_down",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 78, 79, 80, /*81, 82,*/ 83, 84, 85, 86, 87 }, frameTimeAttack, textureManager);

        m_animation->add("assets/player.png", "attack_up",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 88, 89, 90, /*91, 92,*/ 93, 94, 95, 96 }, frameTimeAttack, textureManager);

        m_animation->add("assets/player.png", "hurt",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 37, 38, 39, 40 }, 0.075f, textureManager);

        m_animation->add("assets/player.png", "die",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 69, 70, 71, 72, 73, 74, 75, 76, 77 }, 0.15f, textureManager);
    }

    // Movement
    {
        m_animation->add("assets/player.png", "right",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 25, 26, 27, 28, 29 }, 0.08f, textureManager);

        m_animation->add("assets/player.png", "left",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 20, 21, 22, 23, 24 }, 0.08f, textureManager);

        m_animation->add("assets/player.png", "up",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 15, 16, 17, 18, 19 }, 0.08f, textureManager);

        m_animation->add("assets/player.png", "down",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 0, 1, 2, 3, 4 }, 0.08f, textureManager);
    }

    // Movement diagonally
    {
        m_animation->add("assets/player.png", "down_right",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 10, 11, 12, 13, 14 }, 0.08f, textureManager);

        m_animation->add("assets/player.png", "down_left",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 5, 6, 7, 8, 9 }, 0.08f, textureManager);
    }

    // Dash
    {
        m_animation->add("assets/player.png", "dash_left",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 41, 42, 43, 44, 45 }, 0.1f, textureManager);

        m_animation->add("assets/player.png", "dash_right",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 46, 47, 48, 49, 50 }, 0.1f, textureManager);

        m_animation->add("assets/player.png", "dash_up",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 46, 47, 48, 49, 50 }, 0.1f, textureManager);

        m_animation->add("assets/player.png", "dash_up_left",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 61, 62, 63, 64, 65 }, 0.1f, textureManager);

        m_animation->add("assets/player.png", "dash_up_right",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 56, 57, 58, 59, 60 }, 0.1f, textureManager);

        m_animation->add("assets/player.png", "dash_down",
            jt::Vector2u { static_cast<unsigned int>(GP::PlayerSize().x),
                static_cast<unsigned int>(GP::PlayerSize().y) },
            { 51, 52, 53, 54, 55 }, 0.1f, textureManager);
    }

    m_animation->play("idle");
    m_animation->setPosition(jt::Vector2f { 5 * 24, 7 * 24 });

    m_attackUnderlay = std::make_shared<jt::Animation>();
    m_attackUnderlay->setLooping(false);
    m_attackUnderlay->add("assets/attack_underlay.png", "initial", jt::Vector2u { 32u, 32u }, { 0 },
        0.1f, textureManager);
    m_attackUnderlay->add("assets/attack_underlay.png", "attack", jt::Vector2u { 32u, 32u },
        jt::MathHelper::numbersBetween(3u, 9u), frameTimeAttack * 2.0f, textureManager);
    m_attackUnderlay->play("initial");
}

void PlayerGraphicsComponent::updateGraphics(float elapsed)
{
    m_animation->update(elapsed);
    m_attackUnderlay->update(elapsed);
}

void PlayerGraphicsComponent::setPosition(jt::Vector2f const& playerPosition)
{
    auto spritePosition = playerPosition - GP::PlayerSize() * 0.5f;

    m_animation->setPosition(spritePosition);
    m_attackUnderlay->setPosition(spritePosition - jt::Vector2f { 8.0f, 8.0f });
}

void PlayerGraphicsComponent::draw(std::shared_ptr<jt::RenderTarget> target)
{
    m_attackUnderlay->draw(target);
    m_animation->draw(target);
}

void PlayerGraphicsComponent::flash(float time, jt::Color const& color)
{
    m_animation->flash(time, color);
}

bool PlayerGraphicsComponent::setAnimationIfNotSet(std::string const& newAnimationName)
{
    std::string const currentAnimationName = m_animation->getCurrentAnimationName();

    if (currentAnimationName == "die") {
        return true;
    }

    if (currentAnimationName == "hurt" && newAnimationName == "idle") {
        return true;
    }

    if (currentAnimationName != newAnimationName) {
        m_animation->play(newAnimationName);
        return true;
    }
    return false;
}

void PlayerGraphicsComponent::setPlayerAnimationLooping(bool isLooping)
{
    m_animation->setLooping(isLooping);
}

void PlayerGraphicsComponent::setUnderlayAnimation(std::string const& animationName)
{
    m_attackUnderlay->play(animationName, 0, true);
}
std::string PlayerGraphicsComponent::getCurrentAnimation() const
{
    return m_animation->getCurrentAnimationName();
}
