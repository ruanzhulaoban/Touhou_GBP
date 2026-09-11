#pragma once

#include <QList>
#include "DataStructs.h"

namespace TowerDefense {

/**
 * @brief 所有游戏对象的顶级基类
 * 
 * 提供唯一ID、活跃状态和世界坐标，并规定统一的更新接口。
 */
class GameObject
{
public:
    explicit GameObject(const WorldPos& pos = WorldPos());
    virtual ~GameObject() = default;

    // 禁止拷贝（派生类可使用指针管理）
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    /** 每帧更新逻辑，纯虚函数，由派生类实现 */
    virtual void update(float deltaTime) = 0;

    // ----- 访问器 -----
    int getId() const { return m_id; }
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

    const WorldPos& getWorldPos() const { return m_worldPos; }
    void setWorldPos(const WorldPos& pos) { m_worldPos = pos; }

    // 便捷转换（使用DataStructs中的转换函数）
    GridPos getGridPos(int cellSize = Constants::CELL_SIZE) const {
        return worldToGrid(m_worldPos, cellSize);
    }

protected:
    int m_id;               // 唯一标识符
    bool m_isActive;        // 是否存活（true=有效，false=待销毁）
    WorldPos m_worldPos;    // 像素坐标

    static int s_nextId;    // 全局ID计数器（在cpp中定义）
};

} // namespace TowerDefense