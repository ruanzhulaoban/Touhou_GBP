#include "SlowEffect.h"
#include "../Enemy/EnemyBase.h"

namespace TowerDefense {

SlowEffect::SlowEffect(EnemyBase* target, float duration, float slowPercent)
    : Effect(EffectType::SLOW, duration, target)
    , m_slowPercent(slowPercent)
{
}

void SlowEffect::onApply(EnemyBase* enemy) {
    // 速度由 EnemyBase::calculateCurrentSpeed() 统一计算，
    // 无需在此单独设置，避免与其它减速效果冲突。
    (void)enemy;
}

void SlowEffect::onTick(float deltaTime) {
    m_remainingTime -= deltaTime;
    // 不需要额外操作
}

void SlowEffect::onRemove() {
    // 速度由 EnemyBase::calculateCurrentSpeed() 统一重新计算，
    // 无需在此恢复，避免覆盖其它仍在生效的减速效果。
}

} // namespace TowerDefense