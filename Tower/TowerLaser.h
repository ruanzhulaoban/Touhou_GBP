#pragma once
#include "TowerBase.h"
#include <QVector>

namespace TowerDefense {

class GameManager;

class TowerLaser : public TowerBase {
public:
    explicit TowerLaser(const WorldPos& pos, GameManager* manager);

    QList<EnemyBase*> findTargets() override;
    void performAttack(QList<EnemyBase*> targets) override;
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    // 升级相关
    void doUpgrade() override;

    float getAnimTimer() const { return m_animTimer; }
    float getHitFlashTimer() const { return m_hitFlashTimer; }
    float getLaserTimer() const { return m_laserTimer; }
    float getLaserDuration() const { return m_laserDuration; }

private:
    GameManager* m_gameManager;
    float m_damage;
    float m_laserYThreshold;
    float m_animTimer;
    float m_hitFlashTimer;
    float m_laserTimer;
    float m_laserDuration;

    // 攻击间隔随等级变化
    QVector<float> m_attackIntervals;
    float getAttackIntervalByLevel() const;
};

} // namespace TowerDefense