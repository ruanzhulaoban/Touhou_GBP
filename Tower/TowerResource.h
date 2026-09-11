#pragma once
#include "TowerBase.h"

namespace TowerDefense {

class GameManager;

class TowerResource : public TowerBase {
public:
    explicit TowerResource(const WorldPos& pos, GameManager* manager);

    QList<EnemyBase*> findTargets() override;      // 返回空
    void performAttack(QList<EnemyBase*> targets) override; // 空实现
    void takeDamage(float damage) override;
    void update(float deltaTime) override;

    float getAnimTimer() const { return m_animTimer; }
    float getHitFlashTimer() const { return m_hitFlashTimer; }

private:
    GameManager* m_gameManager;
    float m_produceInterval;
    //int m_produceAmount;
    float m_produceTimer;
    float m_animTimer;
    float m_hitFlashTimer;
    QVector<int> m_produceAmounts;   // 各等级产量（索引 = 等级-1）
    int getProduceAmount() const;    // 根据当前等级返回产量
};

} // namespace TowerDefense