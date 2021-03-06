#ifndef ALAKAJAM14_CRYSTAL_PROJECTILE_HPP
#define ALAKAJAM14_CRYSTAL_PROJECTILE_HPP

#include "animation.hpp"
#include "box2dwrapper/box2d_object.hpp"
#include <memory>

class CrystalProjectile : public jt::Box2DObject {
public:
    CrystalProjectile(
        std::shared_ptr<jt::Box2DWorldInterface> world, b2BodyDef const* def, bool isBoss = false);

    virtual void doCreate();
    virtual void doUpdate(float const elapsed);
    virtual void doDraw() const;

private:
    std::shared_ptr<jt::Animation> m_animation;
    float m_rotation { 0.0f };
    bool m_isBoss;
};

#endif // ALAKAJAM14_CRYSTAL_PROJECTILE_HPP
