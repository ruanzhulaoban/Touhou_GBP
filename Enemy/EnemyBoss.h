#pragma once
#include "EnemyBase.h"
#include "../Effect/Effect.h"

namespace TowerDefense {

class EnemyBoss : public EnemyBase {
public:
    explicit EnemyBoss(const WorldPos& pos);

    void move(float deltaTime) override;
    void takeDamage(float damage, EffectType type = EffectType::NONE) override;
    void onDeath() override;
    void update(float deltaTime) override;

    bool isBoss() const override { return true; }
    int getPhase() const { return m_phase; }

    int getCurrentFrame() const { return m_animFrame; }

private:
    int m_phase;
    float m_phase2DamageMultiplier;
    float m_animTimer;
    int m_animFrame;
};

} // namespace TowerDefense