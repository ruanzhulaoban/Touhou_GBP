#pragma once
#include "Effect.h"

namespace TowerDefense {

class EnemyBase;

class PoisonEffect : public Effect {
public:
    PoisonEffect(EnemyBase* target, float duration, float damagePerTick, float tickInterval);

    void onApply(EnemyBase* enemy) override;
    void onTick(float deltaTime) override;
    void onRemove() override;

private:
    float m_damagePerTick;
    float m_tickInterval;
    float m_tickTimer;
};

} // namespace TowerDefense