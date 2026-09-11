#include "EnemyNormal.h"
#include "../ConfigLoader.h"
#include "../Constants.h"
#include "../Effect/Effect.h"
namespace TowerDefense {

EnemyNormal::EnemyNormal(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::NORMAL,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::NORMAL)),
                ConfigLoader::getEnemySpeed(EnemyType::NORMAL),
                ConfigLoader::getEnemyDamage(EnemyType::NORMAL),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::NORMAL)))
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemyNormal::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemyNormal::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;
    if (m_health <= 0) {
        m_health=0;
        m_isActive = false;
        onDeath();
    }
}

void EnemyNormal::onDeath() {
    // 普通敌人死亡无特殊逻辑
}

void EnemyNormal::update(float deltaTime) {
    EnemyBase::update(deltaTime); // 处理效果、移动
    // 动画更新
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense