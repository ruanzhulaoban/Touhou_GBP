#pragma once
#include "EnemyBase.h"
#include <functional>

namespace TowerDefense {

class EnemySummon : public EnemyBase {
public:
    using SpawnCallback = std::function<void(EnemyType, const GridPos&)>;

    explicit EnemySummon(const WorldPos& pos);
    void setSpawnCallback(SpawnCallback callback) { m_spawnCallback = callback; }

    void move(float deltaTime) override;
    void takeDamage(float damage, EffectType type = EffectType::NONE) override;
    void onDeath() override;
    void update(float deltaTime) override;

    int getCurrentFrame() const { return m_animFrame; }

private:
    int m_spawnCount;
    SpawnCallback m_spawnCallback;
    float m_animTimer;
    int m_animFrame;
};

} // namespace TowerDefense