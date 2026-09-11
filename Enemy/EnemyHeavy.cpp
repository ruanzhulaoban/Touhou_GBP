#include "EnemyHeavy.h"
#include "../ConfigLoader.h"
#include "../Effect/Effect.h"

namespace TowerDefense {

EnemyHeavy::EnemyHeavy(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::HEAVY,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::HEAVY)),
                ConfigLoader::getEnemySpeed(EnemyType::HEAVY),
                ConfigLoader::getEnemyDamage(EnemyType::HEAVY),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::HEAVY)))
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemyHeavy::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemyHeavy::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;
    if (m_health <= 0) {
        m_health=0;
        m_isActive = false;
        onDeath();
    }
}

void EnemyHeavy::onDeath() {
    // 重甲敌人死亡无特殊逻辑
}

void EnemyHeavy::update(float deltaTime) {
    EnemyBase::update(deltaTime);
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense