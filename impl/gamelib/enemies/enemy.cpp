#include "enemy.hpp"
#include "animation.hpp"
#include "audio/sound.hpp"
#include "audio/sound_null.hpp"
#include "bar.hpp"
#include "damage.hpp"
#include "enemies/enemy_ai/ai_state_boss.hpp"
#include "enemies/enemy_ai/ai_state_boss_critical.hpp"
#include "enemies/enemy_ai/ai_state_boss_injured.hpp"
#include "enemies/enemy_ai/ai_state_follow_target.hpp"
#include "enemies/enemy_ai/ai_state_shooter.hpp"
#include "enemies/enemy_ai/ai_state_wait_for_target.hpp"
#include "game_interface.hpp"
#include "game_properties.hpp"
#include "projectile_spawner_interface.hpp"
#include "system_helper.hpp"

Enemy::Enemy(
    std::shared_ptr<jt::Box2DWorldInterface> world, b2BodyDef const* def, EnemyInfo const& info)
    : Box2DObject { world, def }
{
    m_info = info;
    m_experience = info.experience;
    m_hitpoints = info.hitpoints;
    m_closeCombatDamage = info.closeCombatDamage;
    m_movementSpeed = info.movementSpeed;
    m_isBoss = info.isBoss;
}

void Enemy::doCreate()
{
    m_animation = std::make_shared<jt::Animation>();
    for (auto const& animInfo : m_info.animations) {
        m_animation->add(animInfo.fileName, animInfo.animationName, animInfo.imageSize,
            animInfo.frameIndices, animInfo.frameTimeInSeconds, getGame()->gfx().textureManager());
    }
    m_animation->play(m_info.animations.begin()->animationName);

    b2FixtureDef fixtureDef;
    b2CircleShape circle {};
    circle.m_radius = m_info.colliderRadius;

    fixtureDef.shape = &circle;
    fixtureDef.filter.categoryBits = GP::PhysicsCollisionCategoryEnemies();
    fixtureDef.filter.maskBits = GP::PhysicsCollisionCategoryWalls()
        | GP::PhysicsCollisionCategoryPlayer() | GP::PhysicsCollisionCategoryPlayerShots()
        | GP::PhysicsCollisionCategoryEnemies();
    if (isBoss()) {
        fixtureDef.density = 999999999.0f;
    }
    getB2Body()->CreateFixture(&fixtureDef);

    for (auto const& aiInfo : m_info.ais) {
        if (aiInfo.type == aiInfo.WAIT) {
            auto waitState = std::make_shared<AiStateWaitForTarget>();
            waitState->setTarget(m_target);
            waitState->setNextState(aiInfo.nextState);
            waitState->setDetectRange(aiInfo.range);
            getAiStateManager().registerState(aiInfo.name, waitState);
        } else if (aiInfo.type == aiInfo.SHOOT) {
            auto shooterState = std::make_shared<AiStateShooter>();
            shooterState->setTarget(m_target);
            shooterState->setNextState(aiInfo.nextState);
            shooterState->setForgetRange(aiInfo.range);
            shooterState->setProjectileSpawner(m_projectileSpawner);
            getAiStateManager().registerState(aiInfo.name, shooterState);
        } else if (aiInfo.type == aiInfo.FOLLOW) {
            auto followState = std::make_shared<AiStateFollowTarget>();
            followState->setTarget(m_target);
            followState->setNextState(aiInfo.nextState);
            followState->setPathCalculator(m_pathCalculator);
            getAiStateManager().registerState(aiInfo.name, followState);
        } else if (aiInfo.type == aiInfo.BOSS) {

            auto bossState = std::make_shared<AiStateBoss>();
            bossState->setTarget(m_target);
            bossState->setNextState(aiInfo.nextState);
            bossState->setPathCalculator(m_pathCalculator);
            bossState->setProjectileSpawner(m_projectileSpawner);
            getAiStateManager().registerState(aiInfo.name, bossState);
        } else if (aiInfo.type == aiInfo.BOSS_INJURED) {
            auto bossState = std::make_shared<AiStateBossInjured>();
            bossState->setTarget(m_target);
            bossState->setPathCalculator(m_pathCalculator);
            bossState->setNextState(aiInfo.nextState);
            bossState->setProjectileSpawner(m_projectileSpawner);
            getAiStateManager().registerState(aiInfo.name, bossState);
        } else if (aiInfo.type == aiInfo.BOSS_CRITICAL) {
            auto bossState = std::make_shared<AiStateBossCritical>();
            bossState->setTarget(m_target);
            bossState->setPathCalculator(m_pathCalculator);
            bossState->setNextState(aiInfo.nextState);
            bossState->setProjectileSpawner(m_projectileSpawner);
            getAiStateManager().registerState(aiInfo.name, bossState);
        }
    }
    getAiStateManager().switchToState(m_info.ais.begin()->name);

    if (m_isBoss) {
        m_bar = std::make_shared<jt::Bar>(200.0f, 16.0f, true, getGame()->gfx().textureManager());
        m_bar->setIgnoreCamMovement(true);
        m_bar->setPosition(jt::Vector2f {
            GP::GetScreenSize().x - 200.0f - 8.0f, GP::GetScreenSize().y - 16.0f - 8.0f });
        m_bar->setFrontColor(jt::Color { 136, 14, 79 });
        m_bar->setBackColor(jt::Color { 20, 20, 20 });

        m_soundShattering1 = std::make_shared<jt::Sound>("assets/sound/enemy_shattering_boss.ogg");
        m_soundShattering1->setVolume(0.25f);
        getGame()->audio().addTemporarySound(m_soundShattering1);
        m_soundShatteringGroup = std::make_shared<jt::SoundGroup>();
        m_soundShatteringGroup->add(m_soundShattering1);

    } else {
        m_soundShattering1 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering1",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_medium_1.ogg");
            },
            5);
        m_soundShattering1->setVolume(0.5f);

        m_soundShattering2 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering2",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_medium_2.ogg");
            },
            5);
        m_soundShattering2->setVolume(0.5f);

        m_soundShattering3 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering3",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_medium_3.ogg");
            },
            5);
        m_soundShattering3->setVolume(0.5f);

        m_soundShattering4 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering4",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_small_1.ogg");
            },
            5);
        m_soundShattering4->setVolume(0.5f);

        m_soundShattering5 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering5",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_small_2.ogg");
            },
            5);
        m_soundShattering5->setVolume(0.5f);

        m_soundShattering6 = getGame()->audio().getSoundFromSoundPool(
            "enemy_shattering6",
            []() {
                return std::make_shared<jt::Sound>("assets/sound/enemy_shattering_small_3.ogg");
            },
            5);
        m_soundShattering6->setVolume(0.5f);
        m_soundShatteringGroup = std::make_shared<jt::SoundGroup>();
        m_soundShatteringGroup->add(m_soundShattering1);
        m_soundShatteringGroup->add(m_soundShattering2);
        m_soundShatteringGroup->add(m_soundShattering3);
        m_soundShatteringGroup->add(m_soundShattering4);
        m_soundShatteringGroup->add(m_soundShattering5);
        m_soundShatteringGroup->add(m_soundShattering6);
    }

    m_soundHit1 = getGame()->audio().getSoundFromSoundPool(
        "enemy_hit1",
        []() { return std::make_shared<jt::Sound>("assets/sound/enemy_was_hit-001.ogg"); }, 5);
    m_soundHit1->setVolume(0.25f);

    m_soundHit2 = getGame()->audio().getSoundFromSoundPool(
        "enemy_hit2",
        []() { return std::make_shared<jt::Sound>("assets/sound/enemy_was_hit-002.ogg"); }, 5);
    m_soundHit2->setVolume(0.25f);

    m_soundHit3 = getGame()->audio().getSoundFromSoundPool(
        "enemy_hit3",
        []() { return std::make_shared<jt::Sound>("assets/sound/enemy_was_hit-003.ogg"); }, 5);
    m_soundHit3->setVolume(0.25f);

    m_soundHit4 = getGame()->audio().getSoundFromSoundPool(
        "enemy_hit4",
        []() { return std::make_shared<jt::Sound>("assets/sound/enemy_was_hit-004.ogg"); }, 5);
    m_soundHit4->setVolume(0.25f);

    m_soundHitGroup = std::make_shared<jt::SoundGroup>();
    m_soundHitGroup->add(m_soundHit1);
    m_soundHitGroup->add(m_soundHit2);
    m_soundHitGroup->add(m_soundHit3);
    m_soundHitGroup->add(m_soundHit4);
}

void Enemy::doUpdate(const float elapsed)
{
    m_staggeredTime -= elapsed;
    m_attackCooldown -= elapsed;

    m_animation->update(elapsed);
    if (!m_animation->isVisible()) {
        return;
    }

    if (!m_isInDieAnimation) {
        if (m_staggeredTime <= 0) {
            performAI(elapsed);
        }
        m_animation->setPosition(getPosition()
            - jt::Vector2f { m_animation->getLocalBounds().width,
                  m_animation->getLocalBounds().height }
                * 0.5f);

    } else {
        m_animation->setPosition(m_deathPosition
            - jt::Vector2f { m_animation->getLocalBounds().width,
                  m_animation->getLocalBounds().height }
                * 0.5f);
    }
    if (isBoss()) {
        m_bar->setMaxValue(m_info.hitpoints);
        m_bar->setCurrentValue(m_hitpoints);
        m_bar->update(elapsed);
    }
}

void Enemy::doDraw() const { m_animation->draw(getGame()->gfx().target()); }

void Enemy::drawHud() const
{
    if (m_isBoss) {
        if (m_hitpoints != m_info.hitpoints) {
            m_bar->draw(getGame()->gfx().target());
        }
    }
}

void Enemy::receiveDamage(Damage const& dmg)
{
    // TODO visual candy
    m_animation->flash(0.2f, jt::Color { 163, 51, 255 });
    m_soundHitGroup->play();

    m_hitpoints -= dmg.value;

    m_staggeredTime = 0.25f;
    setVelocity({ 0.0f, 0.0f });
    if (m_hitpoints <= 0.0f) {
        die();
    }
}

void Enemy::die()
{
    // don't die twice
    if (m_isInDieAnimation) {
        return;
    }

    m_isInDieAnimation = true;
    m_soundShatteringGroup->play();
    m_animation->play("dead");
    m_deathPosition = getPosition();
    // move collider out of the way
    setPosition(jt::Vector2f { -9999999999.0f, -999999999999999.0f });

    if (m_deferredActionHandler == nullptr) {
        return;
    }

    m_deferredActionHandler->queueDeferredAction(m_animation->getCurrentAnimTotalTime(), [this]() {
        kill();
        if (m_experienceSpawner) {
            m_experienceSpawner->spawnExperience(m_experience, m_deathPosition, false);
        }
    });
}

void Enemy::setTarget(std::weak_ptr<TargetInterface> target) { m_target = target; }
void Enemy::setPathCalculator(WorldPathCalculatorInterface* calculator)
{
    m_pathCalculator = calculator;
}

void Enemy::setProjectileSpawner(ProjectileSpawnerInterface* spawner)
{
    m_projectileSpawner = spawner;
}

void Enemy::performAI(float elapsed)
{
    if (jt::SystemHelper::is_uninitialized_weak_ptr(m_target) || m_target.expired()) {
        return;
    }
    getAiStateManager().update();
    if (getAiStateManager().getCurrentState() == nullptr) {
        return;
    }

    auto currentState = getAiStateManager().getCurrentState();
    currentState->setPosition(getPosition());
    currentState->update(elapsed, this);
}

void Enemy::setDeferredActionHandler(DeferredActionInterface* handler)
{
    m_deferredActionHandler = handler;
}

void Enemy::setExperienceSpawner(ExperienceSpawnerInterface* spawner)
{
    m_experienceSpawner = spawner;
}

AiStateManager& Enemy::getAiStateManager() { return m_aiStateManager; }
void Enemy::moveInDirection(jt::Vector2f const& dir) { setVelocity(dir * m_movementSpeed); }
float Enemy::playAnimation(std::string const& animName)
{
    m_animation->play(animName);
    return m_animation->getCurrentAnimTotalTime();
}
float Enemy::getCloseCombatDamage() const { return m_closeCombatDamage; }
jt::Vector2f Enemy::getTargetPosition() { return getPosition(); }
void Enemy::applyDamageToTarget(Damage const& dmg) { receiveDamage(dmg); }
void Enemy::gainExperience(int value)
{ /* noting to do*/
}
bool Enemy::isBoss() { return m_isBoss; }
void Enemy::makeSpellAvailable(std::string const& spellName) { }
float Enemy::getHitpoints() const { return m_hitpoints; }

EnemyInfo const& Enemy::getInfo() const { return m_info; }
