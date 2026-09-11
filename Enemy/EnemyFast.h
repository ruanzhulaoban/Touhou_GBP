#pragma once
#include "EnemyBase.h"

namespace TowerDefense {

class EnemyFast : public EnemyBase {
public:
    explicit EnemyFast(const WorldPos& pos);

    void move(float deltaTime) override;
    void takeDamage(float damage, EffectType type = EffectType::NONE) override;
    void onDeath() override;
    void update(float deltaTime) override;

    int getCurrentFrame() const { return m_animFrame; }

private:
    float m_animTimer;
    int m_animFrame;
};

} // namespace TowerDefense