#include "GameObject.h"

namespace TowerDefense {

int GameObject::s_nextId = 1;   // 从1开始，0可作为无效ID

GameObject::GameObject(const WorldPos& pos)
    : m_id(s_nextId++)
    , m_isActive(true)
    , m_worldPos(pos)
{
}

} // namespace TowerDefense