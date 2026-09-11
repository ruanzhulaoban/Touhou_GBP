#include "TowerBase.h"

namespace TowerDefense {

TowerBase::TowerBase(const WorldPos& pos, TowerType type,
                     float maxHealth, float attackInterval, float range, int cost)
    : GameObject(pos)
    , m_health(maxHealth)
    , m_maxHealth(maxHealth)
    , m_attackInterval(attackInterval)
    , m_attackCooldown(0.0f)
    , m_range(range)
    , m_cost(cost)
    , m_type(type)
    , m_isWall(false)
    , m_upgradeLevel(1)
    , m_upgradeCost(0)
    , m_maxUpgradeLevel(3)
{
}

TowerBase::~TowerBase()
{
}

void TowerBase::update(float deltaTime)
{
    if (!m_isActive || m_health <= 0) {
        return;
    }
    // 缩减冷却时间
    if (m_attackCooldown > 0) {
        m_attackCooldown -= deltaTime;
        if (m_attackCooldown < 0) m_attackCooldown = 0;
    }
}

void TowerBase::doUpgrade() {
    if (!canUpgrade()) return;
    m_upgradeLevel++;
    // 可在此调用虚函数 onUpgrade() 供子类重写以提升属性
}

} // namespace TowerDefense