#pragma once
#include "TowerBase.h"

namespace TowerDefense {

class GameManager;

class TowerAOE : public TowerBase {
public:
    explicit TowerAOE(const WorldPos& pos, GameManager* manager);

    QList<EnemyBase*> findTargets() override;
    void performAttack(QList<EnemyBase*> targets) override;
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    float getAnimTimer() const { return m_animTimer; }
    float getHitFlashTimer() const { return m_hitFlashTimer; }

    // ---- 环形法阵动画接口 ----
    float getRingTimer() const { return m_ringTimer; }
    float getRingDuration() const { return m_ringDuration; }
    float getRange() const { return m_range; }

    bool hasActiveSkill() const override ; // 二级解锁
    bool isSkillReady() const override ;
    void activateSkill() override;
    // 新增：获取当前等级的技能范围
    float getCurrentSkillRange() const;


private:
    GameManager* m_gameManager;
    float m_damage;
    float m_animTimer;
    float m_hitFlashTimer;

    // 环形法阵动画
    float m_ringTimer;      // 当前剩余时间
    float m_ringDuration;   // 总持续时间

    // 技能相关
    float m_skillCooldown;
    float m_skillCooldownMax;
    bool m_skillReady;
    QVector<float> m_skillRanges;   // 存储各等级技能范围

};

} // namespace TowerDefense