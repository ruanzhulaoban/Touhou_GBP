#pragma once
#include "EnemyBase.h"

namespace TowerDefense {

class EnemyNormal : public EnemyBase {
public:
    explicit EnemyNormal(const WorldPos& pos);

    // 实现纯虚函数
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