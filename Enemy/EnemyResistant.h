#pragma once
#include "EnemyBase.h"

namespace TowerDefense {

class EnemyResistant : public EnemyBase {
public:
    explicit EnemyResistant(const WorldPos& pos);

    void move(float deltaTime) override;
    void takeDamage(float damage, EffectType type = EffectType::NONE) override;
    void applyEffect(Effect* effect) override;
    void onDeath() override;
    void update(float deltaTime) override;

    int getCurrentFrame() const { return m_animFrame; }

private:
    float m_slowResistance; // 减速抗性系数（0~1）
    float m_animTimer;
    int m_animFrame;
};

} // namespace TowerDefense