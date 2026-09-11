#pragma once
#include "../Bullet/Bullet.h"
#include "../Effect/SlowEffect.h"

namespace TowerDefense {

class SlowBullet : public Bullet {
public:
    SlowBullet(const WorldPos& pos, float speed, float damage, float slowPercent, float slowDuration);
    ~SlowBullet() override = default;

protected:
    void checkCollisions(float step) override;

private:
    float m_slowPercent;
    float m_slowDuration;
};

} // namespace TowerDefense