#include "EnemyBase.h"
#include "../Effect/Effect.h"
#include <algorithm>

namespace TowerDefense {

EnemyBase::EnemyBase(const WorldPos& pos, EnemyType type,
                     float maxHealth, float baseSpeed, int damage,float attackInterval)
    : GameObject(pos)
    , m_health(maxHealth)
    , m_maxHealth(maxHealth)
    , m_baseSpeed(baseSpeed)
    , m_currentSpeed(baseSpeed)
    , m_damage(damage)
    , m_type(type)
    , m_isBlocked(false)
    ,m_terrainSpeedMultiplier(1.0f)
    , m_attackInterval(attackInterval)
    , m_attackCooldown(0.0f)
{
}


void EnemyBase::update(float deltaTime) {
    if (!m_isActive || m_health <= 0) return;

    // 1. 更新效果计时
    updateEffects(deltaTime);
    updateAttackCooldown(deltaTime);      // 递减冷却

    // 2. 重新计算当前速度（基于基础速度和生效的减速效果）
    m_currentSpeed = calculateCurrentSpeed();

    // 3. 应用地形倍率（若未被阻塞）
    if (!m_isBlocked) {
        m_currentSpeed *= m_terrainSpeedMultiplier;
    } else {
        m_currentSpeed = 0.0f;
        return;  // 阻塞时不移动
    }

    // 4. 正常移动
    move(deltaTime);
}

void EnemyBase::clearEffects()
{
    for (Effect* effect : m_activeEffects) {
        effect->onRemove();
    }
    m_activeEffects.clear();
}

void EnemyBase::applyEffect(Effect* effect) {
    if (!effect) return;
    // 检查是否已存在同类型效果
    for (Effect* existing : m_activeEffects) {
        if (existing->getType() == effect->getType()) {
            // 刷新该效果的持续时间（同时更新 duration，以支持抗性敌人修改后的时长）
            existing->setDuration(effect->getDuration());
            delete effect;  // 删除新效果，因为已有
            return;
        }
    }
    // 无同类型，添加新效果
    m_activeEffects.append(effect);
    effect->onApply(this);
}

void EnemyBase::updateEffects(float deltaTime) {
    auto it = m_activeEffects.begin();
    while (it != m_activeEffects.end()) {
        Effect* effect = *it;
        // 调用 onTick 更新计时
        effect->onTick(deltaTime);
        // 如果效果过期，移除并删除
        if (effect->isExpired()) {
            effect->onRemove();    // 恢复状态
            delete effect;         // 释放内存
            it = m_activeEffects.erase(it);
        } else {
            ++it;
        }
    }
}

// 在析构函数中清理所有效果（如果尚未实现）
EnemyBase::~EnemyBase() {
    // 清理所有效果（调用 onRemove 并 delete）
    for (Effect* effect : m_activeEffects) {
        effect->onRemove();
        delete effect;
    }
    m_activeEffects.clear();
}

bool EnemyBase::hasEffect(EffectType type) const {
    for (Effect* effect : m_activeEffects) {
        if (effect->getType() == type) {
            return true;
        }
    }
    return false;
}

float EnemyBase::calculateCurrentSpeed() const {
    float totalSlow = 0.0f;
    for (Effect* effect : m_activeEffects) {
        if (effect->getType() == EffectType::SLOW) {
            totalSlow += effect->getSlowPercent();
        }
    }
    if (totalSlow > 1.0f) totalSlow = 1.0f;
    return m_baseSpeed * (1.0f - totalSlow);
}

} // namespace TowerDefense