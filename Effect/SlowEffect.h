#pragma once
#include "Effect.h"

namespace TowerDefense {

// 前向声明
class EnemyBase;

class SlowEffect : public Effect {
public:
    SlowEffect(EnemyBase* target, float duration, float slowPercent);

    void onApply(EnemyBase* enemy) override;
    void onTick(float deltaTime) override;
    void onRemove() override;
    float getSlowPercent() const override { return m_slowPercent; }

private:
    float m_slowPercent;
};

} // namespace TowerDefense