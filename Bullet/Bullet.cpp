#include "Bullet.h"
#include "../Enemy/EnemyBase.h"
#include "../Constants.h"
#include <cmath>
#include <Qdebug>

namespace TowerDefense {

Bullet::Bullet(const WorldPos& pos, float speed, float damage,
               int penetration, float collisionThreshold)
    : GameObject(pos)
    , m_speed(speed)
    , m_damage(damage)
    , m_penetration(penetration)
    , m_maxPenetration(penetration)
    , m_collisionThreshold(collisionThreshold)
    , m_maxTravel(Constants::COLS * Constants::CELL_SIZE * 2.0f)
    , m_xTraveled(0.0f)
    , m_enemyList(nullptr)
    ,m_gameManager(nullptr)
{
}

Bullet::~Bullet()
{
}

void Bullet::update(float deltaTime)
{
    if (!m_isActive) {
        return;
    }

    // 水平向右移动
    float step = m_speed * deltaTime;
    m_worldPos.x += step;
    m_xTraveled += step;

    // 检查是否超出地图右侧或飞行距离上限
    if (m_worldPos.x > (Constants::COLS * Constants::CELL_SIZE + 100) || m_xTraveled > m_maxTravel) {
        m_isActive = false;
        return;
    }

    // 碰撞检测（如果有敌人列表指针）
    if (m_enemyList) {
        checkCollisions(step);
    }

    // 如果穿透次数已耗尽，销毁子弹
    if (m_penetration <= 0) {
        m_isActive = false;
    }
}

void Bullet::checkCollisions(float step)
{
    if (!m_enemyList) {
        qWarning() << "Bullet: m_enemyList is NULL!";
        return;
    }
    const float HIT_RADIUS = 100.0f;   // 增大碰撞半径
    for (EnemyBase* enemy : *m_enemyList) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        int enemyId = enemy->getId();
        if (m_hitEnemyIds.contains(enemyId)) continue;

        const WorldPos& enemyPos = enemy->getWorldPos();
        float dx = enemyPos.x - m_worldPos.x;
        float dy = std::abs(enemyPos.y - m_worldPos.y);

        // 调试输出（可临时开启）
        // qDebug() << "Bullet" << m_id << "checking enemy" << enemyId << "dx=" << dx << "dy=" << dy;

        if (dx > 0 && dy < m_collisionThreshold && dx < HIT_RADIUS) {
            qDebug() << "Bullet" << m_id << "hit enemy" << enemyId << "damage" << m_damage;
            enemy->takeDamage(m_damage, EffectType::NONE);
            m_hitEnemyIds.insert(enemyId);
            m_penetration--;
            if (m_penetration <= 0) break;
        }
    }
}

} // namespace TowerDefense