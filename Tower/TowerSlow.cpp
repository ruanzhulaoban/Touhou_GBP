#include "TowerSlow.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"
#include "../Effect/Effect.h"
#include "../Bullet/SlowBullet.h"
#include <algorithm>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>

namespace TowerDefense {

TowerSlow::TowerSlow(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::SLOW,
                0.0f,  // 临时值，稍后覆盖
                0.0f,
                ConfigLoader::getTowerRange(TowerType::SLOW),
                ConfigLoader::getTowerCost(TowerType::SLOW))
    , m_gameManager(manager)
    , m_slowPercent(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "slowPercent", 0.5)))
    , m_slowDuration(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "slowDuration", 2.0)))
    , m_animTimer(0.0f)
    , m_hitFlashTimer(0.0f)
{
    // ---- 读取攻击间隔数组 ----
    QJsonObject extra = ConfigLoader::getTowerExtraParams(TowerType::SLOW);
    QJsonArray intervalsArray = extra.value("attackIntervals").toArray();
    if (!intervalsArray.isEmpty()) {
        for (const auto& val : intervalsArray) {
            m_attackIntervals.append(static_cast<float>(val.toDouble()));
        }
    } else {
        // 兼容旧配置：使用单一的 attackInterval
        float singleInterval = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "attackInterval", 1.5));
        m_attackIntervals = { singleInterval };
    }
    if (m_attackIntervals.isEmpty()) m_attackIntervals = { 1.5f };
    // 确保至少3个等级
    while (m_attackIntervals.size() < 3) {
        m_attackIntervals.append(m_attackIntervals.last());
    }

    // 设置当前等级的攻击间隔
    m_attackInterval = getAttackIntervalByLevel();
    m_attackCooldown = 0.0f;

    // 读取升级费用
    m_upgradeCost = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "upgradeCost", 30));
    m_maxUpgradeLevel = 3;

    // 从 JSON 读取生命值（基类未设置，需手动设置）
    m_maxHealth = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "health", 100.0));
    m_health = m_maxHealth;
}

QList<EnemyBase*> TowerSlow::findTargets() {
    QList<EnemyBase*> candidates;
    float halfCell = Constants::CELL_SIZE * 0.4f;
    int myRow = getGridPos().row;

    // 只查塔所在行（dy 阈值 < 半格，跨行必然超出）
    const QList<EnemyBase*>& rowEnemies = m_gameManager->getEnemiesInRow(myRow);
    for (EnemyBase* enemy : rowEnemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        const WorldPos& ePos = enemy->getWorldPos();
        float dx = ePos.x - m_worldPos.x;
        float dy = std::abs(ePos.y - m_worldPos.y);
        if (dx >= -50.0f && dy < halfCell && dx < m_range) {
            candidates.append(enemy);
        }
    }

    if (candidates.isEmpty()) return candidates;
    std::sort(candidates.begin(), candidates.end(),
              [](EnemyBase* a, EnemyBase* b) {
                  return a->getWorldPos().x < b->getWorldPos().x;
              });
    return { candidates.first() };
}

void TowerSlow::performAttack(QList<EnemyBase*> targets) {
    if (targets.isEmpty()) return;
    EnemyBase* target = targets.first();

    float damage = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "damage", 10.0));
    float bulletSpeed = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::SLOW, "bulletSpeed", 200.0));

    // 生成位置偏移（与 ArrowTower 类似）
    WorldPos bulletPos = m_worldPos;
    bulletPos.x += Constants::CELL_SIZE / 2.0f - 50.0f;   // 或者尝试只偏移半格

    SlowBullet* bullet = new SlowBullet(bulletPos, bulletSpeed, damage, m_slowPercent, m_slowDuration);
    bullet->setEnemyList(&m_gameManager->getEnemies());
    m_gameManager->addBullet(bullet);

    m_animTimer = 0.3f;
    resetCooldown();
}

void TowerSlow::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerSlow::update(float deltaTime) {
    TowerBase::update(deltaTime);
    if (m_animTimer > 0) m_animTimer -= deltaTime;
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;

    if (canAttack() && !isDead()) {
        QList<EnemyBase*> targets = findTargets();
        if (!targets.isEmpty()) {
            performAttack(targets);
        }
    }
}

// ---- 升级相关 ----
float TowerSlow::getAttackIntervalByLevel() const {
    int idx = m_upgradeLevel - 1;
    if (idx < 0) idx = 0;
    if (idx >= m_attackIntervals.size()) idx = m_attackIntervals.size() - 1;
    return m_attackIntervals[idx];
}

void TowerSlow::doUpgrade() {
    if (!canUpgrade()) return;
    TowerBase::doUpgrade();   // 等级+1
    m_attackInterval = getAttackIntervalByLevel();
    // 若当前冷却大于新间隔，重置为0，避免等待过久
    if (m_attackCooldown > m_attackInterval) {
        m_attackCooldown = 0.0f;
    }
}

} // namespace TowerDefense