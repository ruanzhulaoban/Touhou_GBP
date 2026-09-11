#pragma once
#include "TowerBase.h"

namespace TowerDefense {

class GameManager;

class TowerSlow : public TowerBase {
public:
    explicit TowerSlow(const WorldPos& pos, GameManager* manager);

    QList<EnemyBase*> findTargets() override;
    void performAttack(QList<EnemyBase*> targets) override;
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    // 升级相关
    void doUpgrade() override;

    float getAnimTimer() const { return m_animTimer; }
    float getHitFlashTimer() const { return m_hitFlashTimer; }

private:
    GameManager* m_gameManager;
    float m_slowPercent;
    float m_slowDuration;
    float m_animTimer;
    float m_hitFlashTimer;
    QVector<float> m_attackIntervals;   // 各等级攻击间隔
    float getAttackIntervalByLevel() const;
};

} // namespace TowerDefense