#include "EnemySummon.h"
#include "../ConfigLoader.h"
#include "../Constants.h"
#include "../Effect/Effect.h"

namespace TowerDefense {

EnemySummon::EnemySummon(const WorldPos& pos)
    : EnemyBase(pos, EnemyType::SUMMON,
                static_cast<float>(ConfigLoader::getEnemyHealth(EnemyType::SUMMON)),
                ConfigLoader::getEnemySpeed(EnemyType::SUMMON),
                ConfigLoader::getEnemyDamage(EnemyType::SUMMON),
                static_cast<float>(ConfigLoader::getEnemyAttackInterval(EnemyType::SUMMON)))
    , m_spawnCount(static_cast<int>(ConfigLoader::getEnemyExtraParam(EnemyType::SUMMON, "summonSpawnCount", 2.0)))
    , m_spawnCallback(nullptr)
    , m_animTimer(0.0f)
    , m_animFrame(0)
{
}

void EnemySummon::move(float deltaTime) {
    m_worldPos.x -= m_currentSpeed * deltaTime;
}

void EnemySummon::takeDamage(float damage, EffectType type) {
    (void)type;
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
        onDeath();
    }
}

void EnemySummon::onDeath() {
    if (m_spawnCallback) {
        GridPos deathPos = this->getGridPos();
        // 计算右侧相邻列，若超出地图边界则限制为最后一列
        int spawnCol = deathPos.col + 1;
        if (spawnCol >= Constants::COLS) {
            spawnCol = Constants::COLS - 1;  // 最多到最后一列
        }
        for (int i = 0; i < m_spawnCount; ++i) {
            GridPos spawnGrid(deathPos.row, spawnCol);
            m_spawnCallback(EnemyType::NORMAL, spawnGrid);
        }
    }
}

void EnemySummon::update(float deltaTime) {
    EnemyBase::update(deltaTime);
    m_animTimer += deltaTime;
    if (m_animTimer >= 0.15f) {
        m_animTimer = 0.0f;
        m_animFrame = (m_animFrame + 1) % 4;
    }
}

} // namespace TowerDefense