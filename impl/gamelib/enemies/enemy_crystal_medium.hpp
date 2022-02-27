#ifndef ALAKAJAM14_ENEMY_CRYSTAL_MEDIUM_HPP
#define ALAKAJAM14_ENEMY_CRYSTAL_MEDIUM_HPP

#include "enemy_base.hpp"

class EnemyCrystalMedium : public EnemyBase {
public:
    EnemyCrystalMedium(
        std::shared_ptr<jt::Box2DWorldInterface> world, b2BodyDef const* def, StateGame& state);

private:
    void doAI(float elapsed) override;
    void doCreate() override;
};

#endif // ALAKAJAM14_ENEMY_CRYSTAL_MEDIUM_HPP