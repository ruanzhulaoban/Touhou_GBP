#include "Effect.h"

namespace TowerDefense {

Effect::Effect(EffectType type, float duration, EnemyBase* target)
    : m_type(type)
    , m_duration(duration)
    , m_remainingTime(duration)
    , m_target(target)
{
}


} // namespace TowerDefense