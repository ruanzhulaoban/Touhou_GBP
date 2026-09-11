#include "TowerLaser.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"
#include "../Effect/BurnEffect.h"
#include <algorithm>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>

namespace TowerDefense {

TowerLaser::TowerLaser(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::LASER,
                0.0f,  // 临时值，稍后覆盖
                0.0f,
                ConfigLoader::getTowerRange(TowerType::LASER),
                ConfigLoader::getTowerCost(TowerType::LASER))
    , m_gameManager(manager)
    , m_damage(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "damage", 15.0)))
    , m_laserYThreshold(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "laserYThreshold", 15.0)))
    , m_animTimer(0.0f)
    , m_hitFlashTimer(0.0f)
    , m_laserTimer(0.0f)
    , m_laserDuration(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "laserDuration", 0.3f)))
{
    // ---- 读取攻击间隔数组 ----
    QJsonObject extra = ConfigLoader::getTowerExtraParams(TowerType::LASER);
    QJsonArray intervalsArray = extra.value("attackIntervals").toArray();
    if (!intervalsArray.isEmpty()) {
        for (const auto& val : intervalsArray) {
            m_attackIntervals.append(static_cast<float>(val.toDouble()));
        }
    } else {
        // 兼容旧配置：使用单一的 attackInterval
        float singleInterval = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "attackInterval", 1.0));
        m_attackIntervals = { singleInterval };
    }
    if (m_attackIntervals.isEmpty()) m_attackIntervals = { 1.0f };
    // 确保至少3个等级
    while (m_attackIntervals.size() < 3) {
        m_attackIntervals.append(m_attackIntervals.last());
    }

    // 设置当前等级的攻击间隔
    m_attackInterval = getAttackIntervalByLevel();
    m_attackCooldown = 0.0f;

    // 读取升级费用
    m_upgradeCost = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "upgradeCost", 40));
    m_maxUpgradeLevel = 3;

    // 从 JSON 读取生命值（基类未设置，需手动设置）
    m_maxHealth = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "health", 100.0));
    m_health = m_maxHealth;
}

QList<EnemyBase*> TowerLaser::findTargets() {
    QList<EnemyBase*> targets;
    float halfThreshold = m_laserYThreshold;
    int myRow = getGridPos().row;

    // 只查塔所在行（dy 阈值远小于一格高度，跨行必然超出）
    const QList<EnemyBase*>& rowEnemies = m_gameManager->getEnemiesInRow(myRow);
    for (EnemyBase* enemy : rowEnemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        const WorldPos& ePos = enemy->getWorldPos();
        float dx = ePos.x - m_worldPos.x;
        float dy = std::abs(ePos.y - m_worldPos.y);
        if (dx > 0 && dy < halfThreshold && dx < m_range) {
            targets.append(enemy);
        }
    }

    std::sort(targets.begin(), targets.end(),
              [](EnemyBase* a, EnemyBase* b) {
                  return a->getWorldPos().x < b->getWorldPos().x;
              });
    return targets;
}

void TowerLaser::performAttack(QList<EnemyBase*> targets) {
    if (targets.isEmpty()) return;

    float damagePerTick = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "burnDamage", 12.0));
    float tickInterval = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "burnTickInterval", 0.3));
    float duration = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::LASER, "burnDuration", 2.0));

    for (EnemyBase* enemy : targets) {
        Effect* burn = new BurnEffect(enemy, duration, damagePerTick, tickInterval);
        enemy->applyEffect(burn);
    }

    m_laserTimer = m_laserDuration;
    m_animTimer = 0.2f;
    resetCooldown();
}

void TowerLaser::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerLaser::update(float deltaTime) {
    TowerBase::update(deltaTime);
    if (m_animTimer > 0) m_animTimer -= deltaTime;
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;
    if (m_laserTimer > 0) m_laserTimer -= deltaTime;

    if (canAttack() && !isDead()) {
        QList<EnemyBase*> targets = findTargets();
        if (!targets.isEmpty()) {
            performAttack(targets);
        }
    }
}

// ---- 升级相关 ----
float TowerLaser::getAttackIntervalByLevel() const {
    int idx = m_upgradeLevel - 1;
    if (idx < 0) idx = 0;
    if (idx >= m_attackIntervals.size()) idx = m_attackIntervals.size() - 1;
    return m_attackIntervals[idx];
}

void TowerLaser::doUpgrade() {
    if (!canUpgrade()) return;
    TowerBase::doUpgrade();   // 等级+1
    m_attackInterval = getAttackIntervalByLevel();
    // 若当前冷却大于新间隔，重置为0
    if (m_attackCooldown > m_attackInterval) {
        m_attackCooldown = 0.0f;
    }
}

} // namespace TowerDefense