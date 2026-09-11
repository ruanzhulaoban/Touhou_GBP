#pragma once
#include "TowerBase.h"
#include <QVector>

namespace TowerDefense {

class GameManager;

class TowerWall : public TowerBase {
public:
    explicit TowerWall(const WorldPos& pos, GameManager* manager);

    QList<EnemyBase*> findTargets() override;
    void performAttack(QList<EnemyBase*> targets) override;
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    // 升级相关
    void doUpgrade() override;   // ← 添加声明

    float getHitFlashTimer() const { return m_hitFlashTimer; }

private:
    GameManager* m_gameManager;
    float m_hitFlashTimer;
    QVector<float> m_wallHealths;   // 各等级生命值
    float getWallHealth() const;    // 根据当前等级获取生命值
};

} // namespace TowerDefense