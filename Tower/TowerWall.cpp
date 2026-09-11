#include "TowerWall.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"
#include <QJsonArray>
#include <QJsonObject>

namespace TowerDefense {

TowerWall::TowerWall(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::WALL,
                0.0f,  // 临时，稍后覆盖
                0.0f,
                0.0f,
                ConfigLoader::getTowerCost(TowerType::WALL))
    , m_gameManager(manager)
    , m_hitFlashTimer(0.0f)
{
    m_isWall = true;

    // 读取生命值数组
    QJsonObject extra = ConfigLoader::getTowerExtraParams(TowerType::WALL);
    QJsonArray healthsArray = extra.value("wallHealths").toArray();
    if (!healthsArray.isEmpty()) {
        for (const auto& val : healthsArray) {
            m_wallHealths.append(static_cast<float>(val.toDouble()));
        }
    } else {
        // 兼容旧配置
        float singleHealth = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::WALL, "wallHealth", 400.0));
        m_wallHealths = { singleHealth };
    }
    if (m_wallHealths.isEmpty()) m_wallHealths = { 400.0f };
    // 确保至少3个等级
    while (m_wallHealths.size() < 3) {
        m_wallHealths.append(m_wallHealths.last());
    }

    m_maxHealth = getWallHealth();
    m_health = m_maxHealth;

    // 读取升级费用
    m_upgradeCost = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::WALL, "upgradeCost", 20));
    m_maxUpgradeLevel = 3;
}

QList<EnemyBase*> TowerWall::findTargets() {
    return {};
}

void TowerWall::performAttack(QList<EnemyBase*> targets) {
    // 空实现
}

void TowerWall::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerWall::update(float deltaTime) {
    if (isDead()) return;  // 防御墙无攻击冷却，仅需存活检查
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;
}

float TowerWall::getWallHealth() const {
    int idx = m_upgradeLevel - 1;
    if (idx < 0) idx = 0;
    if (idx >= m_wallHealths.size()) idx = m_wallHealths.size() - 1;
    return m_wallHealths[idx];
}

void TowerWall::doUpgrade() {
    if (!canUpgrade()) return;
    TowerBase::doUpgrade();   // 增加等级
    float newHealth = getWallHealth();
    m_maxHealth = newHealth;
    m_health = newHealth;     // 升级回满血
}

} // namespace TowerDefense