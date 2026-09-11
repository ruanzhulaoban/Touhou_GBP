#include "PoisonEffect.h"
#include "../Enemy/EnemyBase.h"

namespace TowerDefense {

PoisonEffect::PoisonEffect(EnemyBase* target, float duration, float damagePerTick, float tickInterval)
    : Effect(EffectType::POISON, duration, target)
    , m_damagePerTick(damagePerTick)
    , m_tickInterval(tickInterval)
    , m_tickTimer(0.0f)
{
}

void PoisonEffect::onApply(EnemyBase* enemy) {
    (void)enemy;
    m_tickTimer = 0.0f;
}

void PoisonEffect::onTick(float deltaTime) {
    m_remainingTime -= deltaTime;
    m_tickTimer += deltaTime;
    if (m_tickTimer >= m_tickInterval && m_target && !m_target->isDead()) {
        m_tickTimer = 0.0f;
        m_target->takeDamage(m_damagePerTick, EffectType::POISON);
    }
}

void PoisonEffect::onRemove() {
    // 中毒无恢复操作
}

} // namespace TowerDefense