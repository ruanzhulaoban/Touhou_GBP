#pragma once

#include <QSet>
#include "../GameObject.h"

namespace TowerDefense {

// 前向声明
class EnemyBase;
class GameManager;

/**
 * @brief 子弹类
 * 
 * 不锁定目标，水平向右飞行，每帧检测与敌人的碰撞。
 * 支持穿透（可命中多个敌人），记录已命中ID避免重复伤害。
 */
class Bullet : public GameObject
{
public:
    /**
     * @param pos 起始世界坐标
     * @param speed 飞行速度（像素/秒）
     * @param damage 单次伤害值
     * @param penetration 穿透次数（0表示不穿透，普通子弹为0或1；激光可大于1）
     * @param collisionThreshold 碰撞检测Y轴容差（像素）
     */
    Bullet(const WorldPos& pos, float speed, float damage,
           int penetration = 0, float collisionThreshold = 20.0f);
    virtual ~Bullet() override;

    Bullet(const Bullet&) = delete;
    Bullet& operator=(const Bullet&) = delete;

    /** @brief 每帧移动并检测碰撞，需要传入当前所有存活敌人列表 */
    virtual void update(float deltaTime) override;

    /** @brief 设置敌人列表指针（由GameManager在更新时传入） */
    void setEnemyList(const QList<EnemyBase*>* enemies) { m_enemyList = enemies; }

    // ----- 访问器 -----
    float getDamage() const { return m_damage; }
    bool isExpired() const { return !m_isActive || m_xTraveled > m_maxTravel; }

    void setGameManager(GameManager* manager) { m_gameManager = manager; }
protected:
    /** @brief 碰撞检测（step为本帧移动步长） */
    virtual void checkCollisions(float step);

    float m_speed;           // 飞行速度（像素/秒）
    float m_damage;          // 伤害值
    int m_penetration;       // 剩余穿透次数（每次命中减1）
    int m_maxPenetration;    // 初始穿透次数（用于记录）
    float m_collisionThreshold; // Y轴碰撞容差（像素）

    float m_maxTravel;       // 最大飞行距离（像素）
    float m_xTraveled;       // 已飞行的距离

    QSet<int> m_hitEnemyIds; // 已命中敌人的ID集合

    const QList<EnemyBase*>* m_enemyList; // 敌人列表指针
    GameManager* m_gameManager;   // 由 GameManager 设置
};

} // namespace TowerDefense