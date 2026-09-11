#include "EnemyResistant.h"
#include "../ConfigLoader.h"
#include "../Effect/Effect.h"

namespace TowerDefense {

EnemyResistant::EnemyResistant(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::RESISTANT,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::RESISTANT)),
                ConfigLoader::getEnemySpeed(EnemyType::RESISTANT),
                ConfigLoader::getEnemyDamage(EnemyType::RESISTANT),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::RESISTANT)))
    , m_slowResistance(static_cast<float>(ConfigLoader::getEnemySlowResistance(EnemyType::RESISTANT)))
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemyResistant::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemyResistant::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
        onDeath();
    }
}

void EnemyResistant::applyEffect(Effect* effect) {
    if (!effect) return;
    // 减速抗性：缩短持续时间
    if (effect->getType() == EffectType::SLOW) {
        effect->setDuration(effect->getDuration() * m_slowResistance);
    }
    // 委托基类处理去重和添加
    EnemyBase::applyEffect(effect);
}

void EnemyResistant::onDeath() {
    // 抗性敌人死亡无特殊逻辑
}

void EnemyResistant::update(float deltaTime) {
    EnemyBase::update(deltaTime);
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense