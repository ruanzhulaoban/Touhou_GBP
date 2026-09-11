#pragma once

#include <QList>
#include "../GameObject.h"
#include "../Enums.h"

namespace TowerDefense {

// 前向声明
class EnemyBase;

/**
 * @brief 防御塔抽象基类
 * 
 * 所有塔类型（射手、减速、AOE、激光、资源、防御墙）均从此派生。
 * 塔可被敌人攻击并摧毁。
 */
class TowerBase : public GameObject
{
public:
    TowerBase(const WorldPos& pos, TowerType type,
              float maxHealth, float attackInterval, float range, int cost);
    virtual ~TowerBase() override;

    TowerBase(const TowerBase&) = delete;
    TowerBase& operator=(const TowerBase&) = delete;

    /** @brief 每帧更新：缩减冷却，派生类需调用基类 */
    virtual void update(float deltaTime) override;

    /** @brief 纯虚：查找攻击目标 */
    virtual QList<EnemyBase*> findTargets() = 0;

    /** @brief 纯虚：执行攻击（针对targets列表） */
    virtual void performAttack(QList<EnemyBase*> targets) = 0;

    /** @brief 纯虚：受到敌人伤害 */
    virtual void takeDamage(float damage) = 0;

    // ----- 普通方法 -----
    bool isDead() const { return m_health <= 0 || !m_isActive; }
    bool canAttack() const { return m_attackCooldown <= 0 && m_isActive && m_health > 0; }

    /** 重置冷却（用于攻击后调用） */
    void resetCooldown() { m_attackCooldown = m_attackInterval; }

    // ----- 访问器 -----
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    float getAttackInterval() const { return m_attackInterval; }
    float getRange() const { return m_range; }
    int getCost() const { return m_cost; }
    TowerType getType() const { return m_type; }
    bool isWall() const { return m_isWall; }
    void setWall(bool wall) { m_isWall = wall; }

    int getUpgradeLevel() const { return m_upgradeLevel; }
    int getUpgradeCost() const { return m_upgradeCost; }
    bool canUpgrade() const { return m_upgradeLevel < m_maxUpgradeLevel; }
    virtual void doUpgrade();   // 执行升级，子类可重写
    // 主动技能接口
    virtual bool hasActiveSkill() const { return false; }
    virtual bool isSkillReady() const { return false; }
    virtual void activateSkill() {}   // 空实现

protected:
    float m_health;
    float m_maxHealth;

    float m_attackInterval;   // 攻击间隔（秒）
    float m_attackCooldown;   // 当前剩余冷却时间（秒）
    float m_range;            // 攻击范围半径（像素）
    int m_cost;               // 建造费用

    TowerType m_type;
    bool m_isWall;            // 是否为防御墙（特殊阻挡逻辑）
    int m_upgradeLevel = 1;
    int m_upgradeCost = 0;
    int m_maxUpgradeLevel = 3;   // 默认最大3级
};

} // namespace TowerDefense