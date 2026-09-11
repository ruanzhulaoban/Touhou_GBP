#pragma once

#include "../Enums.h"

namespace TowerDefense {

// 前向声明
class EnemyBase;

/**
 * @brief 状态效果基类
 * 
 * 用于实现减速、中毒、灼烧三种效果。
 * 效果被附加到EnemyBase上，由EnemyBase驱动更新。
 */
class Effect
{
public:
    Effect(EffectType type, float duration, EnemyBase* target);
    virtual ~Effect() = default;

    // 禁止拷贝
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;

    /** @brief 附加到目标时调用（用于初始化效果参数） */
    virtual void onApply(EnemyBase* enemy) = 0;

    /** @brief 每帧调用（由EnemyBase驱动），处理计时和持续伤害 */
    virtual void onTick(float deltaTime) = 0;

    /** @brief 移除效果时调用（恢复被修改的属性） */
    virtual void onRemove() = 0;

    // ----- 访问器 -----
    EffectType getType() const { return m_type; }
    float getDuration() const { return m_duration; }
    virtual void setDuration(float duration) { m_duration = duration; m_remainingTime = duration; }
    float getRemainingTime() const { return m_remainingTime; }
    bool isExpired() const { return m_remainingTime <= 0; }
    EnemyBase* getTarget() const { return m_target; }

    // 用于设置参数的接口（子类可重写）
    virtual void setSlowPercent(float percent) {}
    virtual void setDamagePerTick(float damage) {}
    virtual void setTickInterval(float interval) {}

    virtual float getSlowPercent() const { return 0.0f; }
    virtual void reset() { m_remainingTime = m_duration; }  // 重置剩余时间

protected:
    EffectType m_type;
    float m_duration;        // 总持续时间（秒）
    float m_remainingTime;   // 剩余时间（秒）
    EnemyBase* m_target;     // 作用目标（原始指针，由GameManager管理）
};

} // namespace TowerDefense