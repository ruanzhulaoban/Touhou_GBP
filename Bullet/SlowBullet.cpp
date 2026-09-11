#include "SlowBullet.h"
#include "../Enemy/EnemyBase.h"
#include "../Effect/SlowEffect.h"
#include "../GameManager.h"

namespace TowerDefense {

SlowBullet::SlowBullet(const WorldPos& pos, float speed, float damage, float slowPercent, float slowDuration)
    : Bullet(pos, speed, damage, 1, 20.0f)   // 穿透1次
    , m_slowPercent(slowPercent)
    , m_slowDuration(slowDuration)
{
}

void SlowBullet::checkCollisions(float step) {
    if (!m_enemyList) return;
    const float HIT_RADIUS = 30.0f;
    for (EnemyBase* enemy : *m_enemyList) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        if (m_hitEnemyIds.contains(enemy->getId())) continue;

        const WorldPos& ePos = enemy->getWorldPos();
        float dx = ePos.x - m_worldPos.x;
        float dy = std::abs(ePos.y - m_worldPos.y);

        if (dx > 0 && dy < m_collisionThreshold && dx < HIT_RADIUS) {
            // 获取地形，增强减速
            float actualSlowPercent = m_slowPercent;
            if (m_gameManager) {
                GridPos pos = enemy->getGridPos();
                if (pos.isValid() && m_gameManager->getMapTerrain(pos.row, pos.col) == TerrainType::ICE) {
                    actualSlowPercent = qMin(1.0f, m_slowPercent * 1.5f); // 冰上减速增强 1.5 倍
                }
            }
            // 造成伤害
            enemy->takeDamage(m_damage, EffectType::NONE);
            // 施加减速
            Effect* effect = new SlowEffect(enemy, m_slowDuration, actualSlowPercent);
            enemy->applyEffect(effect);

            m_hitEnemyIds.insert(enemy->getId());
            m_penetration--;
            if (m_penetration <= 0) break;
        }
    }
}

} // namespace TowerDefense