#include "TowerAOE.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"
#include "../Effect/PoisonEffect.h"
#include "../Audio/AudioManager.h"
#include <cmath>

namespace TowerDefense {

TowerAOE::TowerAOE(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::AOE,
                static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "health", 100.0)),
                ConfigLoader::getTowerAttackInterval(TowerType::AOE),
                ConfigLoader::getTowerRange(TowerType::AOE),
                ConfigLoader::getTowerCost(TowerType::AOE))
    , m_gameManager(manager)
    , m_damage(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "damage", 30.0)))
    , m_animTimer(0.0f)
    , m_hitFlashTimer(0.0f)
    , m_ringTimer(0.0f)   // 新增
    , m_ringDuration(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "ringDuration", 0.6f))) // 从JSON读取，默认0.6秒
    , m_skillCooldown(0.0f)
    , m_skillCooldownMax(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "skillCooldown", 30.0f)))
    , m_skillReady(true)
{
    // 从JSON读取升级费用
    m_upgradeCost = ConfigLoader::getTowerExtraParam(TowerType::AOE, "upgradeCost", 80);
    // 读取技能范围数组（若不存在则使用默认值）
    QJsonArray skillRangesArray = ConfigLoader::getTowerExtraParams(TowerType::AOE).value("skillRanges").toArray();
    if (skillRangesArray.isEmpty()) {
        // 默认为单一范围，与攻击范围相同
        m_skillRanges = { m_range };
    } else {
        for (const auto& val : skillRangesArray) {
            m_skillRanges.append(static_cast<float>(val.toDouble()));
        }
    }
}

QList<EnemyBase*> TowerAOE::findTargets() {
    QList<EnemyBase*> targets;
    float radiusSq = m_range * m_range;
    int myRow = getGridPos().row;
    // 上取整：(range + CELL_SIZE - 1) / CELL_SIZE，确保不漏边界敌人
    int rowRadius = (static_cast<int>(m_range) + Constants::CELL_SIZE - 1)
                    / Constants::CELL_SIZE;

    for (int dr = -rowRadius; dr <= rowRadius; ++dr) {
        int r = myRow + dr;
        if (r < 0 || r >= Constants::ROWS) continue;
        const QList<EnemyBase*>& rowEnemies = m_gameManager->getEnemiesInRow(r);
        for (EnemyBase* enemy : rowEnemies) {
            if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
            const WorldPos& ePos = enemy->getWorldPos();
            float dx = ePos.x - m_worldPos.x;
            float dy = ePos.y - m_worldPos.y;
            if ((dx*dx + dy*dy) < radiusSq) {
                targets.append(enemy);
            }
        }
    }
    return targets;
}

void TowerAOE::performAttack(QList<EnemyBase*> targets) {
    if (targets.isEmpty()) return;

    float damagePerTick = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "poisonDamage", 8.0));
    float tickInterval = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "poisonTickInterval", 0.5));
    float duration = static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::AOE, "poisonDuration", 3.0));

    for (EnemyBase* enemy : targets) {
        Effect* poison = new PoisonEffect(enemy, duration, damagePerTick, tickInterval);
        enemy->applyEffect(poison);
    }

    // 保留动画触发
    m_animTimer = 0.5f;
    m_ringTimer = m_ringDuration;
    resetCooldown();
}

void TowerAOE::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerAOE::update(float deltaTime) {
    TowerBase::update(deltaTime);
    if (m_animTimer > 0) m_animTimer -= deltaTime;
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;
    if (m_ringTimer > 0) m_ringTimer -= deltaTime;   // 递减

    // 技能冷却
    if (!m_skillReady) {
        m_skillCooldown -= deltaTime;
        if (m_skillCooldown <= 0) {
            m_skillReady = true;
            m_skillCooldown = 0;
        }
    }

    if (canAttack() && !isDead()) {
        QList<EnemyBase*> targets = findTargets();
        if (!targets.isEmpty()) {
            performAttack(targets);
        }
    }
}

void TowerAOE::activateSkill() {
    if (!m_skillReady || !hasActiveSkill()) return;

    float skillRange = getCurrentSkillRange();
    const QList<EnemyBase*>& enemies = m_gameManager->getEnemies();
    for (EnemyBase* enemy : enemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        const WorldPos& ePos = enemy->getWorldPos();
        float dx = ePos.x - m_worldPos.x;
        float dy = ePos.y - m_worldPos.y;
        // 只对距离小于技能范围的敌人生效
        if ((dx*dx + dy*dy) < skillRange * skillRange) {
            enemy->takeDamage(9999.0f, EffectType::NONE);   // 秒杀
        }
    }
    // 播放技能音效
    AudioManager::instance()->playSoundEffect(
        QStringLiteral("./assets/audio/sfx_aoe_skill.mp3"), 0.7f);

    m_skillReady = false;
    m_skillCooldown = m_skillCooldownMax;
}

bool TowerAOE::hasActiveSkill() const {
    return m_upgradeLevel >= 2;   // 假设2级解锁技能
}

bool TowerAOE::isSkillReady() const {
    return m_skillReady;
}

float TowerAOE::getCurrentSkillRange() const {
    int levelIndex = m_upgradeLevel - 1;   // 等级从1开始，索引0对应等级1
    if (m_skillRanges.isEmpty()) return m_range;   // 兜底
    if (levelIndex >= m_skillRanges.size()) {
        return m_skillRanges.last();   // 超出则用最后一个
    }
    return m_skillRanges[levelIndex];
}

} // namespace TowerDefense