#include "quint.hpp"

namespace jt {
namespace ease {
namespace quint {

float easeIn(float t, float b, float c, float d) { return c * (t /= d) * t * t * t * t + b; }

float easeOut(float t, float b, float c, float d)
{
    return c * ((t = t / d - 1) * t * t * t * t + 1) + b;
}

float easeInOut(float t, float b, float c, float d)
{
    if ((t /= d / 2) < 1)
        return c / 2 * t * t * t * t * t + b;
    return c / 2 * ((t -= 2) * t * t * t * t + 2) + b;
}

} // namespace quint
} // namespace ease
} // namespace jt
