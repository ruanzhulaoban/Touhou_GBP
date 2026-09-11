#include "EnemyFast.h"
#include "../ConfigLoader.h"
#include "../Effect/Effect.h"

namespace TowerDefense {

EnemyFast::EnemyFast(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::FAST,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::FAST)),
                ConfigLoader::getEnemySpeed(EnemyType::FAST),
                ConfigLoader::getEnemyDamage(EnemyType::FAST),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::FAST)))
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemyFast::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemyFast::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;
    if (m_health <= 0) {
        m_health=0;
        m_isActive = false;
        onDeath();
    }
}

void EnemyFast::onDeath() {
    // 快速敌人死亡无特殊逻辑
}

void EnemyFast::update(float deltaTime) {
    EnemyBase::update(deltaTime);
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense