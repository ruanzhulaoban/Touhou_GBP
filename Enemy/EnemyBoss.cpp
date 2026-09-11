#include "EnemyBoss.h"
#include "../ConfigLoader.h"
#include <QDebug>

namespace TowerDefense {

EnemyBoss::EnemyBoss(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::BOSS,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::BOSS)),
                ConfigLoader::getEnemySpeed(EnemyType::BOSS),
                ConfigLoader::getEnemyDamage(EnemyType::BOSS),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::BOSS)))
    , m_phase(1)
    , m_phase2DamageMultiplier(static_cast<float>(
          ConfigLoader::getEnemyExtraParam(EnemyType::BOSS, "bossPhase2DamageMultiplier", 1.5)))
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemyBoss::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemyBoss::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;

    if (m_health <= 0) {
        if (m_phase == 1) {
            // 第一阶段结束，进入第二阶段
            m_phase = 2;
            m_health = m_maxHealth;  // 回满血
            // 伤害增强
            m_damage = static_cast<int>(static_cast<float>(m_damage) * m_phase2DamageMultiplier);
            // 触发阶段切换回调（可选）
            onPhaseChange();
            qDebug() << "Boss enters phase 2, damage increased to" << m_damage;
        } else {
            // 第二阶段结束，真正死亡
            m_health=0;
            m_isActive = false;
            onDeath();
        }
    }
}

void EnemyBoss::onDeath() {
    // Boss死亡逻辑（可在此触发胜利等，由外部处理）
}

void EnemyBoss::update(float deltaTime) {
    EnemyBase::update(deltaTime);
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense