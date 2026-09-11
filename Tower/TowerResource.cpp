#include "TowerResource.h"
#include "../GameManager.h"
#include "../ConfigLoader.h"

namespace TowerDefense {

TowerResource::TowerResource(const WorldPos& pos, GameManager* manager)
    : TowerBase(pos, TowerType::RESOURCE,
                static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::RESOURCE, "health", 100.0)),
                ConfigLoader::getTowerAttackInterval(TowerType::RESOURCE), // 不使用
                ConfigLoader::getTowerRange(TowerType::RESOURCE),          // 不使用
                ConfigLoader::getTowerCost(TowerType::RESOURCE))
    , m_gameManager(manager)
    , m_produceInterval(static_cast<float>(ConfigLoader::getTowerExtraParam(TowerType::RESOURCE, "produceInterval", 3.0)))
    , m_produceTimer(0.0f)
    , m_animTimer(0.0f)
    , m_hitFlashTimer(0.0f)
{
    // 从 JSON 读取升级费用
    m_upgradeCost = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::RESOURCE, "upgradeCost", 50));
    // 读取产量数组
    QJsonObject extra = ConfigLoader::getTowerExtraParams(TowerType::RESOURCE);
    QJsonArray amountsArray = extra.value("produceAmounts").toArray();
    if (!amountsArray.isEmpty()) {
        for (const auto& val : amountsArray) {
            m_produceAmounts.append(val.toInt());
        }
    } else {
        // 兼容旧配置：使用单个 produceAmount
        int singleAmount = static_cast<int>(ConfigLoader::getTowerExtraParam(TowerType::RESOURCE, "produceAmount", 10));
        m_produceAmounts = { singleAmount };
    }
    // 确保至少有一个值
    if (m_produceAmounts.isEmpty()) m_produceAmounts = { 10 };
}

QList<EnemyBase*> TowerResource::findTargets() {
    return {}; // 空列表
}

void TowerResource::performAttack(QList<EnemyBase*> targets) {
    // 空实现
}

void TowerResource::takeDamage(float damage) {
    m_health -= damage;
    m_hitFlashTimer = 0.1f;
    if (m_health <= 0) {
        m_health = 0;
        m_isActive = false;
    }
}

void TowerResource::update(float deltaTime) {
    if (m_animTimer > 0) m_animTimer -= deltaTime;
    if (m_hitFlashTimer > 0) m_hitFlashTimer -= deltaTime;
    // 资源塔无攻击冷却，跳过 TowerBase::update 的冷却缩减

    // 生产资源
    if (!isDead()) {
        m_produceTimer += deltaTime;
        if (m_produceTimer >= m_produceInterval) {
            m_produceTimer = 0.0f;
            // 调用GameManager增加资源
            int amount=getProduceAmount();
            m_gameManager->addResource(amount); //等级决定增量
            m_animTimer = 0.2f; // 生产闪光
        }
    }
}

int TowerResource::getProduceAmount() const {
    int levelIndex = m_upgradeLevel - 1;   // 等级1 → 索引0
    if (levelIndex < 0) levelIndex = 0;
    if (levelIndex >= m_produceAmounts.size()) {
        return m_produceAmounts.last();   // 超出则用最后一个
    }
    return m_produceAmounts[levelIndex];
}

} // namespace TowerDefense