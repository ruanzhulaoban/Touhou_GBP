#pragma once
#include "TowerBase.h"
#include <Qvector>

namespace TowerDefense {

// 前向声明
class GameManager;

class TowerArrow : public TowerBase {
public:
    explicit TowerArrow(const WorldPos& pos, GameManager* manager);

    // 实现纯虚函数
    QList<EnemyBase*> findTargets() override;
    void performAttack(QList<EnemyBase*> targets) override;
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    // 升级与技能接口
    bool hasActiveSkill() const override { return m_upgradeLevel >= 2; }
    bool isSkillReady() const override { return m_skillReady; }
    void activateSkill() override;

    float getAnimTimer() const { return m_animTimer; }
    float getHitFlashTimer() const { return m_hitFlashTimer; }

private:
    GameManager* m_gameManager;
    float m_damage;
    float m_bulletSpeed;
    float m_animTimer;        // 攻击动画计时
    float m_hitFlashTimer;    // 受击闪烁计时

    // 技能相关
    float m_skillCooldown;
    float m_skillCooldownMax;
    bool m_skillReady;
    QVector<int> m_skillBulletCounts;   // 各等级齐射子弹数（索引=等级-1）
    int getSkillBulletCount() const;    // 根据当前等级获取子弹数
};

} // namespace TowerDefense