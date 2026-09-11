#include "TowerArrow.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"
#include "../Bullet/Bullet.h"
#include <algorithm>
#include <cmath>
#include <QJsonArray>
#include <QJsonObject>

namespace TowerDefense {

TowerArrow::TowerArrow(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::ARROW,
                static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::ARROW, "health", 100.0)),
                ConfigLoader::getTowerAttackInterval(TowerType::ARROW),
                ConfigLoader::getTowerRange(TowerType::ARROW),
                ConfigLoader::getTowerCost(TowerType::ARROW))
    , m_gameManager(manager)
    , m_damage(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::ARROW, "damage", 20.0)))
    , m_bulletSpeed(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::ARROW, "bulletSpeed", 300.0)))
    , m_animTimer(0.0f)
    , m_hitFlashTimer(0.0f)
    , m_skillCooldown(0.0f)
    , m_skillCooldownMax(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::ARROW, "skillCooldown", 20.0f)))
    , m_skillReady(true)
{
    // 读取升级费用
    m_upgradeCost = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::ARROW, "upgradeCost", 50));

    // 读取各等级齐射子弹数
    QJsonObject extra = ConfigLoader::getTowerExtraParams(TowerType::ARROW);
    QJsonArray countsArray = extra.value("skillBulletCounts").toArray();
    if (!countsArray.isEmpty()) {
        for (const auto& val : countsArray) {
            m_skillBulletCounts.append(val.toInt());
        }
    } else {
        // 兼容旧配置：默认等级1无技能，等级2=8发，等级3=12发
        m_skillBulletCounts = {0, 8, 12};   // 索引0对应等级1（无效），等级2=8，等级3=12
    }
    // 确保至少有三个值
    while (m_skillBulletCounts.size() < 3) {
        m_skillBulletCounts.append(8);
    }

}

QList<EnemyBase*> TowerArrow::findTargets() {
    QList<EnemyBase*> candidates;
    float halfCell = Constants::CELL_SIZE * 0.4f;
    int myRow = getGridPos().row;

    // 只查塔所在行（dy 阈值 < 半格，跨行必然超出）
    const QList<EnemyBase*>& rowEnemies = m_gameManager->getEnemiesInRow(myRow);
    for (EnemyBase* enemy : rowEnemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        const WorldPos& ePos = enemy->getWorldPos();
        float dx = ePos.x - m_worldPos.x;
        float dy = std::abs(ePos.y - m_worldPos.y);
        if (dx >=-50.0f && dy < halfCell && dx < m_range) {
            candidates.append(enemy);
        }
    }

    if (candidates.isEmpty()) return candidates;

    // 按 X 从小到大排序（最左优先）
    std::sort(candidates.begin(), candidates.end(),
              [](EnemyBase* a, EnemyBase* b) {
                  return a->getWorldPos().x < b->getWorldPos().x;
              });

    // 只取最左的一个
    return { candidates.first() };
}

void TowerArrow::performAttack(QList<EnemyBase*> targets) {
    if (targets.isEmpty()) return;

    EnemyBase* target = targets.first();

    WorldPos bulletPos = m_worldPos;
    bulletPos.x += Constants::CELL_SIZE / 2.0f-50.0f;
    // 创建子弹
    Bullet* bullet = new Bullet(bulletPos, m_bulletSpeed, m_damage, 1); // 0穿透，击中一次即销毁
    bullet->setEnemyList(&m_gameManager->getEnemies()); // 传入敌人列表引用（但要注意生命周期）
    m_gameManager->addBullet(bullet);

    m_animTimer = 0.3f;
    resetCooldown();
}

void TowerArrow::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerArrow::update(float deltaTime) {
    TowerBase::update(deltaTime); // 冷却缩减
    if (m_animTimer > 0) m_animTimer -= deltaTime;
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;

    // 技能冷却
    if (!m_skillReady) {
        m_skillCooldown -= deltaTime;
        if (m_skillCooldown <= 0) {
            m_skillReady = true;
            m_skillCooldown = 0;
        }
    }
    // 攻击逻辑：如果冷却完成且未死亡
    if (canAttack() && !isDead()) {
        QList<EnemyBase*> targets = findTargets();
        if (!targets.isEmpty()) {
            performAttack(targets);
        }
    }
}

int TowerArrow::getSkillBulletCount() const {
    int idx = m_upgradeLevel - 1;
    if (idx < 0) idx = 0;
    if (idx >= m_skillBulletCounts.size()) {
        return m_skillBulletCounts.last();
    }
    return m_skillBulletCounts[idx];
}

void TowerArrow::activateSkill() {
    if (!m_skillReady || !hasActiveSkill()) return;

    int bulletCount = getSkillBulletCount();
    if (bulletCount <= 0) return;

    // 随机生成子弹，Y方向偏移范围（±30像素）
    const float Y_SPREAD = 30.0f;
    for (int i = 0; i < bulletCount; ++i) {
        // 计算随机偏移
        float yOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * Y_SPREAD;
        WorldPos bulletPos = m_worldPos;
        bulletPos.x += 10.0f;   // 略微前移，避免卡在塔内
        bulletPos.y += yOffset;

        Bullet* bullet = new Bullet(bulletPos, m_bulletSpeed, m_damage, 1); // 穿透1次
        bullet->setEnemyList(&m_gameManager->getEnemies());
        m_gameManager->addBullet(bullet);
    }

    m_skillReady = false;
    m_skillCooldown = m_skillCooldownMax;
    m_animTimer = 0.3f;   // 攻击动画
}

} // namespace TowerDefense