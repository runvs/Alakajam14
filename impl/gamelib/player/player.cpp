#include "player/player.hpp"
#include "enemies/enemy_base.hpp"
#include "game_interface.hpp"
#include "game_properties.hpp"
#include "hud/hud.hpp"
#include "math_helper.hpp"
#include "player_input_component.hpp"
#include "spells/spell_none.hpp"
#include "spells/spell_passive_movement_speed.hpp"
#include "state_game.hpp"

Player::Player(
    std::shared_ptr<jt::Box2DWorldInterface> world, b2BodyDef const* def, StateGame& state)
    : jt::Box2DObject { world, def }
    , m_state { state }
{
}

void Player::doCreate()
{
    b2FixtureDef fixtureDef;
    b2PolygonShape boxCollider {};
    boxCollider.SetAsBox(GP::PlayerSize().x / 2.1f, GP::PlayerSize().y / 2.1f);
    fixtureDef.shape = &boxCollider;
    fixtureDef.filter.categoryBits = GP::PhysicsCollisionCategoryPlayer();
    fixtureDef.filter.maskBits = GP::PhysicsCollisionCategoryWalls()
        | GP::PhysicsCollisionCategoryEnemyShots() | GP::PhysicsCollisionCategoryEnemies();
    getB2Body()->CreateFixture(&fixtureDef);

    createAnimation();

    CharSheetObservers charSheetObservers { m_state.m_hud->getObserverExperience(),
        m_state.m_hud->getObserverHealth(), m_state.m_hud->getObserverHealthMax(), m_healCallback };

    m_charsheet = std::make_shared<CharacterSheetImgui>(charSheetObservers);
    m_charsheet->setGameInstance(getGame());
    m_charsheet->create();

    m_spellBook = std::make_shared<SpellBook>(m_state);
    m_spellBook->setGameInstance(getGame());
    m_spellBook->create();

    createSounds();

    m_commands.push_back(getGame()->getActionCommandManager().registerTemporaryCommand(
        "learnspell", [this](std::vector<std::string> args) {
            getGame()->cheat();
            if (args.size() != 1) {
                return;
            }
            m_spellBook->makeSpellAvailable(args.at(0));
        }));

    m_commands.push_back(getGame()->getActionCommandManager().registerTemporaryCommand(
        "getxp", [this](std::vector<std::string> args) {
            getGame()->cheat();
            if (args.size() != 1) {
                return;
            }
            int amount = std::stoi(args.at(0));
            m_charsheet->changeExperiencePoints(amount);
        }));

    m_input = std::make_shared<PlayerInputComponent>(getGame()->input().keyboard());
}
void Player::createSounds()
{
    m_soundDash = std::make_shared<jt::Sound>("assets/sound/attack_dash_3.ogg");
    m_soundDash->setVolume(0.4f);
    getGame()->audio().addTemporarySound(m_soundDash);

    m_soundStomp = std::make_shared<jt::Sound>("assets/sound/attack_stomp.ogg");
    m_soundStomp->setVolume(0.4f);
    getGame()->audio().addTemporarySound(m_soundStomp);

    m_soundDeath = std::make_shared<jt::Sound>("assets/sound/GAME_OVER.ogg");
    getGame()->audio().addTemporarySound(m_soundDeath);

    auto const soundHurt1 = std::make_shared<jt::Sound>("assets/sound/hit_squishy_sound_01.ogg");
    auto const soundHurt2 = std::make_shared<jt::Sound>("assets/sound/hit_squishy_sound_02.ogg");
    auto const soundHurt3 = std::make_shared<jt::Sound>("assets/sound/hit_squishy_sound_03.ogg");
    auto const soundHurt4 = std::make_shared<jt::Sound>("assets/sound/hit_squishy_sound_04.ogg");
    auto const soundHurt5 = std::make_shared<jt::Sound>("assets/sound/hit_squishy_sound_05.ogg");

    soundHurt1->setVolume(0.6f);
    soundHurt2->setVolume(0.6f);
    soundHurt3->setVolume(0.6f);
    soundHurt4->setVolume(0.6f);
    soundHurt5->setVolume(0.6f);

    m_soundGroupHurt = std::make_shared<jt::SoundGroup>();
    m_soundGroupHurt->add(soundHurt1);
    m_soundGroupHurt->add(soundHurt2);
    m_soundGroupHurt->add(soundHurt3);
    m_soundGroupHurt->add(soundHurt4);
    m_soundGroupHurt->add(soundHurt5);
}



void Player::doUpdate(float const elapsed)
{
    if (m_dashTimer < 0.0f) {
        m_input->updateMovement(*this);
    }

    auto const canAttack = m_attackCooldown < 0.0f && m_dashTimer < 0.0f;
    if (canAttack) {
        m_input->updateAttack(*this);
    }
    m_input->updateSpells(*this);

    handleDash();

    updateSpells(elapsed);
    updateAnimation(elapsed);

    m_dashTimer -= elapsed;
    m_dashCooldown -= elapsed * m_charsheet->getDashFactor();
    m_attackCooldown -= elapsed * m_charsheet->getAttackSpeedFactor();

    m_charsheet->update(elapsed);
    m_spellBook->update(elapsed);

    if (getCharSheet()->getHitpoints() <= 0 && !m_soundDeath->isPlaying() && m_isDying) {
        m_hasFinishedDying = true;
    }
}

void Player::updateSpells(const float elapsed)
{
    auto const& equippedSpells = m_spellBook->getEquippedSpells();
    auto const& equippedSpellsText = m_spellBook->getEquippedSpellTexts();

    for (auto i = 0; i != equippedSpells.size(); ++i) {
        std::shared_ptr<SpellInterface> spell = equippedSpells.at(i);
        std::shared_ptr<jt::Text> text = equippedSpellsText.at(i);
        spell->update(elapsed);
        auto const cost = spell->getExperienceCost();
        auto hasEnoughMana = m_charsheet->getExperiencePoints() >= cost;

        auto spellCanBeCast = spell->canTrigger() && hasEnoughMana;
        if (spellCanBeCast) {
            text->setColor(jt::colors::White);
        } else {
            text->setColor(jt::colors::Gray);
        }
    }
}

void Player::castSpell(std::size_t spellIndex)
{
    std::shared_ptr<SpellInterface> spell = m_spellBook->getEquippedSpells().at(spellIndex);
    auto text = m_spellBook->getEquippedSpellTexts().at(spellIndex);

    auto const cost = spell->getExperienceCost();
    auto hasEnoughMana = m_charsheet->getExperiencePoints() >= cost;
    auto spellCanBeCast = spell->canTrigger() && hasEnoughMana;

    if (spellCanBeCast) {
        getGame()->getLogger().debug("Spell triggered: " + spell->getName());
        m_charsheet->changeExperiencePoints(-cost);
        spell->trigger();
    } else {
        text->shake(0.4f, 2);
    }
}

void Player::updateAnimation(float const elapsed)
{
    auto const notInDashNotInAttack = m_dashTimer <= 0.0f && m_attackCooldown <= 0.0f;
    if (notInDashNotInAttack) {
        auto const walkAnimation = selectWalkAnimation(getVelocity());
        setAnimationIfNotSet(walkAnimation);
    }

    m_animation->setPosition(getPosition() - GP::PlayerSize() * 0.5f);
    m_attackUnderlay->setPosition(
        getPosition() - GP::PlayerSize() * 0.5f - jt::Vector2f { 8.0f, 8.0f });

    m_animation->update(elapsed);
    m_attackUnderlay->update(elapsed);
}

void Player::handleDash()
{
    if (m_dashTimer > 0.0f) {
        auto const isInActiveDashMode = m_dashTimer >= GP::PlayerDashTotalTime()
                - GP::PlayerDashActiveTime() * m_charsheet->getDashFactor();

        if (isInActiveDashMode) {
            setVelocity(m_dashVelocity);
        } else {
            setVelocity(jt::Vector2f { 0.0f, 0.0f });
        }
    }
}

std::string Player::selectDashAnimation(jt::Vector2f const& velocity) const
{
    std::string dashAnimationName { "dash_down" };
    if (velocity.x > 0 && abs(velocity.y) >= 0 && abs(velocity.y) < 0.1f) {
        dashAnimationName = "dash_right";
    } else if (velocity.x < 0 && abs(velocity.y) >= 0 && abs(velocity.y) < 0.1f) {
        dashAnimationName = "dash_left";
    } else if (velocity.x > 0 && velocity.y < 0) {
        dashAnimationName = "dash_up_right";
    } else if (velocity.x < 0 && velocity.y < 0) {
        dashAnimationName = "dash_up_left";
    } else if (abs(velocity.x) >= 0 && abs(velocity.x) < 0.1f && velocity.y < 0) {
        dashAnimationName = "dash_up";
    }
    return dashAnimationName;
}

std::string Player::selectWalkAnimation(jt::Vector2f const& velocity) const
{
    std::string walkAnimationName { "idle" };

    if (jt::MathHelper::lengthSquared(velocity) < 2) {
        walkAnimationName = "idle";
    } else if (abs(velocity.x) > abs(velocity.y)) {
        if (velocity.x > 0) {
            walkAnimationName = "right";
        } else {
            walkAnimationName = "left";
        }
    } else {
        if (velocity.y > 0 && abs(velocity.x) < 0.1f) {
            walkAnimationName = "down";
        } else if (velocity.y > 0 && velocity.x > 0) {
            walkAnimationName = "down_right";
        } else if (velocity.y > 0 && velocity.x < 0) {
            walkAnimationName = "down_left";
        } else {
            walkAnimationName = "up";
        }
    }

    return walkAnimationName;
}

bool Player::setAnimationIfNotSet(std::string const& newAnimationName)
{
    std::string const& currentAnimationName = m_animation->getCurrentAnimationName();

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

void Player::doDraw() const
{
    m_attackUnderlay->draw(getGame()->gfx().target());
    m_animation->draw(getGame()->gfx().target());
    m_charsheet->draw();
    m_spellBook->draw();
}

std::shared_ptr<CharacterSheetImgui> Player::getCharSheet() { return m_charsheet; }

void Player::gainExperience(int value) { m_charsheet->changeExperiencePoints(value); }

void Player::receiveDamage(Damage const& dmg)
{
    m_charsheet->changeHitpoints(dmg.value);
    setAnimationIfNotSet("hurt");
    m_soundGroupHurt->play();
}

void Player::die()
{
    if (!m_isDying) {
        m_isDying = true;
        m_soundDeath->play();
        m_animation->setLooping(false);
        setAnimationIfNotSet("die");
    }
}
std::shared_ptr<SpellBook> Player::getSpellBook() { return m_spellBook; }

jt::Vector2f Player::getTargetPosition() { return getPosition(); }
void Player::applyDamageToTarget(Damage const& dmg) { receiveDamage(dmg); }

void Player::setHealCallback(std::function<void(void)> healCallback)
{
    m_healCallback = [healCallback, this]() {
        m_animation->flash(0.2f, jt::colors::Green);
        healCallback();
    };
}

void Player::dash()
{
    if (m_dashCooldown >= 0) {
        return;
    }

    auto currentPlayerVelocity = getVelocity();

    if (jt::MathHelper::lengthSquared(currentPlayerVelocity) > 0) {
        m_dashTimer = GP::PlayerDashTotalTime();
        m_dashCooldown = GP::PlayerBaseDashCooldown();
    }

    setAnimationIfNotSet(selectDashAnimation(currentPlayerVelocity));

    m_soundDash->stop();
    m_soundDash->play();

    m_animation->flash(0.3f);
    auto p = getPosition();

    jt::MathHelper::normalizeMe(currentPlayerVelocity);
    m_dashVelocity
        = currentPlayerVelocity * GP::PlayerBaseDashVelocity() * m_charsheet->getDashFactor();

    // TODO trigger eye candy (e.g. particles)
}

void Player::attack()
{
    if (!setAnimationIfNotSet("attack_down")) {
        return;
    }

    m_attackCooldown = GP::PlayerAttackCooldown();

    m_soundStomp->stop();
    m_soundStomp->play();

    m_attackUnderlay->play("attack", 0, true);
    // TODO: trigger eye candy (e.g. particles)

    for (auto enemyWeakPtr : *m_state.getEnemies()) {
        auto enemy = enemyWeakPtr.lock();
        if (enemy == nullptr) {
            continue;
        }
        auto nmePos = enemy->getPosition();
        auto myPos = getPosition();
        jt::Vector2f delta = nmePos - myPos;
        auto dist = jt::MathHelper::length(delta);

        float circularHurtboxRange = 20.0f;
        float directedHurtboxRange = 45.0f;

        if (dist < directedHurtboxRange) {
            // Forward-facing hurtbox with medium range
            jt::Vector2f look = getVelocity();
            if (jt::MathHelper::length(look) < 0.1f) {
                // Look down if not moving
                look = { 0.0f, 1.0f };
            }
            jt::MathHelper::normalizeMe(look);
            jt::MathHelper::normalizeMe(delta);
            float sc = jt::MathHelper::dot(look, delta);
            if (sc > 0.5f) {
                enemy->receiveDamage(Damage { m_charsheet->getAttackDamageValue() });
            }
            continue;
        } else if (dist < circularHurtboxRange) {
            // Circular hurtbox with short range
            enemy->receiveDamage(Damage { m_charsheet->getAttackDamageValue() / 3.0f });
        }
    }
}
