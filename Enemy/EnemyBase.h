#pragma once

#include <QList>
#include <QSet>
#include "../GameObject.h"
#include "../Enums.h"

namespace TowerDefense {

// 前向声明
class Effect;

/**
 * @brief 敌人抽象基类
 *
 * 所有敌人类型（普通、快速、重甲、抗性、召唤、Boss）均从此派生。
 * 移动方向固定为水平向左（x递减）。
 */
class EnemyBase : public GameObject
{
public:
    EnemyBase(const WorldPos& pos, EnemyType type,
              float maxHealth, float baseSpeed, int damage,float attackInterval);
    virtual ~EnemyBase() override;

    // 禁止拷贝
    EnemyBase(const EnemyBase&) = delete;
    EnemyBase& operator=(const EnemyBase&) = delete;

    /** @brief 每帧更新：先更新效果，然后根据 m_isBlocked 决定是否移动 */
    virtual void update(float deltaTime) override;

    /** @brief 纯虚：移动逻辑，具体实现应为水平向左移动 */
    virtual void move(float deltaTime) = 0;

    /** @brief 受到伤害，附带伤害类型（用于抗性/特效触发） */
    virtual void takeDamage(float damage, EffectType type = EffectType::NONE) = 0;

    /** @brief 附加状态效果（基类实现含同类型去重，派生类一般无需重写） */
    virtual void applyEffect(Effect* effect);

    /** @brief 死亡回调（用于召唤、阶段切换等，由GameManager处理） */
    virtual void onDeath() = 0;

    /** @brief 是否为Boss（默认false） */
    virtual bool isBoss() const { return false; }

    /** @brief Boss专用：阶段切换回调（空实现） */
    virtual void onPhaseChange() {}

    // ----- 普通方法 -----
    bool isDead() const { return m_health <= 0 || !m_isActive; }

    /** 更新所有附加效果（计时、伤害跳、移除过期效果） */
    void updateEffects(float deltaTime);

    /** 清除所有效果（可用于死亡或重置） */
    void clearEffects();

    /** 设置阻塞状态（由GameManager在碰撞检测中调用） */
    void setBlocked(bool blocked) { m_isBlocked = blocked; }
    bool isBlocked() const { return m_isBlocked; }

    // ----- 访问器 -----
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    float getBaseSpeed() const { return m_baseSpeed; }
    float getCurrentSpeed() const { return m_currentSpeed; }
    void setCurrentSpeed(float speed) { m_currentSpeed = speed; }
    int getDamage() const { return m_damage; }
    EnemyType getType() const { return m_type; }

    bool hasEffect(EffectType type) const;

    bool hasTeleportedGate(const GridPos& pos) const { return m_teleportedGates.contains(pos); }
    void addTeleportedGate(const GridPos& pos) { m_teleportedGates.insert(pos); }
    void clearTeleportedGates() { m_teleportedGates.clear(); } // 重置时清空

    float calculateCurrentSpeed() const;
    void setTerrainSpeedMultiplier(float multiplier) { m_terrainSpeedMultiplier = multiplier; }
    const QList<Effect*>& getActiveEffects() const { return m_activeEffects; }

    float getAttackInterval() const { return m_attackInterval; }
    void resetAttackCooldown() { m_attackCooldown = m_attackInterval; }
    bool canAttack() const { return m_attackCooldown <= 0; }
    void updateAttackCooldown(float delta) { if (m_attackCooldown > 0) m_attackCooldown -= delta; }
protected:
    // 血量
    float m_health;
    float m_maxHealth;

    // 速度（像素/秒）
    float m_baseSpeed;
    float m_currentSpeed;   // 实际速度，受减速效果和阻塞影响

    // 对防御塔的伤害值
    int m_damage;

    EnemyType m_type;

    // 当前附加的状态效果列表（由GameManager统一管理内存）
    QList<Effect*> m_activeEffects;

    // 阻塞标志（当敌人与防御塔碰撞时由GameManager设置）
    bool m_isBlocked;

    QSet<GridPos> m_teleportedGates;   // 已传送过的格子集合
    float m_terrainSpeedMultiplier;   // 地形速度倍率，默认1.0

    float m_attackInterval;
    float m_attackCooldown;
};

} // namespace TowerDefense