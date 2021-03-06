#ifndef ALAKAJAM14_TARGET_INTERFACE_HPP
#define ALAKAJAM14_TARGET_INTERFACE_HPP

#include "damage.hpp"
#include "vector.hpp"

class TargetInterface {
public:
    virtual ~TargetInterface() = default;

    virtual jt::Vector2f getTargetPosition() = 0;

    virtual void applyDamageToTarget(Damage const& dmg) = 0;

    virtual void gainExperience(int value) = 0;

    virtual void makeSpellAvailable(std::string const& spellName) = 0;
};

#endif // ALAKAJAM14_TARGET_INTERFACE_HPP
