#include "spell_heal.hpp"
#include <algorithm>

SpellHeal::SpellHeal(CharacterSheetImgui& charSheet)
    : m_charSheet { charSheet }
{
}

void SpellHeal::trigger()
{
    m_cooldown += 0.5f;
    float amountToHeal
        = std::min(m_charSheet.getHitpointsMax() - m_charSheet.getHitpoints(), 30.0f);
    m_charSheet.changeHitpoints(-amountToHeal);
}

bool SpellHeal::canTrigger() const
{
    bool const hasLessThanFullHitpoints
        = m_charSheet.getHitpoints() < m_charSheet.getHitpointsMax();
    return m_cooldown <= 0.0f && hasLessThanFullHitpoints;
}
void SpellHeal::update(float elapsed) { m_cooldown -= elapsed; }
int SpellHeal::getExperienceCost() const { return 15.0f; }
void SpellHeal::onEquip() { }
void SpellHeal::onUnEquip() { }
std::string SpellHeal::getName() const { return "Heal"; }
