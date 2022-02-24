#include "shake_impl.hpp"
#include "random/random.hpp"

namespace jt {

void ShakeImpl::updateShake(float elapsed)
{
    if (m_shakeTimer > 0) {
        if (m_shakeInterval <= 0) {
            m_shakeInterval = m_shakeIntervalMax;
            auto const currentShakeStrength = m_shakeTimer / m_shakeTimerMax * m_shakeStrength;
            m_shakeOffset.x = jt::Random::getFloat(-currentShakeStrength, currentShakeStrength);
            m_shakeOffset.y = jt::Random::getFloat(-currentShakeStrength, currentShakeStrength);
        }
        m_shakeTimer -= elapsed;
        m_shakeInterval -= elapsed;

    } else {
        m_shakeOffset.x = m_shakeOffset.y = 0;
    }
}
void ShakeImpl::doShake(float t, float strength, float shakeInterval)
{
    m_shakeTimer = t;
    m_shakeTimerMax = t;
    m_shakeStrength = strength;
    m_shakeInterval = m_shakeIntervalMax = shakeInterval;
}
jt::Vector2f ShakeImpl::doGetShakeOffset() const { return m_shakeOffset; }

} // namespace jt
