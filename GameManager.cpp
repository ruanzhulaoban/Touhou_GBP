#include "GameManager.h"
#include "ConfigLoader.h"
#include <QDateTime>
#include <QDebug>

// 包含具体塔和敌人类型（后续阶段会生成这些头文件）

#include "Tower/TowerArrow.h"
#include "Tower/TowerSlow.h"
#include "Tower/TowerAOE.h"
#include "Tower/TowerLaser.h"
#include "Tower/TowerResource.h"
#include "Tower/TowerWall.h"
#include "Enemy/EnemyNormal.h"
#include "Enemy/EnemyFast.h"
#include "Enemy/EnemyHeavy.h"
#include "Enemy/EnemyResistant.h"
#include "Enemy/EnemySummon.h"
#include "Enemy/EnemyBoss.h"
#include "Audio/AudioManager.h"

namespace TowerDefense {

// ---------- 构造与析构 ----------
GameManager::GameManager(QObject *parent)
    : QObject(parent)
    , m_gameMap(nullptr)
    , m_levelManager(nullptr)
    , m_resource(0)
    , m_currentWave(0)
    , m_waveTimer(0.0f)
    , m_enemiesSpawnedInWave(0)
    , m_waveInProgress(false)
    , m_gamePaused(false)
    , m_gameOver(false)
    , m_gameWin(false)
    , m_deltaTime(0.0f)
    , m_lastTime(0)
{
    m_enemiesByRow.resize(Constants::ROWS);
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameManager::onTimerTimeout);
    // 设定帧间隔 ~60 FPS（16ms）
    m_gameTimer->setInterval(16);
    m_elapsedTime = 0.0f;
}

GameManager::~GameManager()
{
    shutdownGame();
    // QTimer 由 Qt 父对象自动释放
}

// ---------- 初始化与生命周期 ----------
bool GameManager::initGame(int levelIndex)
{
    // 清理之前的状态
    shutdownGame();

    // 创建关卡管理器
    m_levelManager = new LevelManager();
    if (!m_levelManager->loadLevel(levelIndex)) {
        qWarning() << "GameManager: Failed to load level" << levelIndex;
        delete m_levelManager;
        m_levelManager = nullptr;
        return false;
    }

    // 创建地图管理器
    m_gameMap = new GameMap();
    if (!m_gameMap->loadMap(levelIndex)) {
        qWarning() << "GameManager: Failed to load map for level" << levelIndex;
        delete m_gameMap;
        m_gameMap = nullptr;
        delete m_levelManager;
        m_levelManager = nullptr;
        return false;
    }

    // 设置初始资源
    m_resource = m_levelManager->getInitialResource();
    emit resourceChanged(m_resource);

    // 重置波次状态
    m_currentWave = 0;
    emit waveChanged(m_currentWave);
    m_waveTimer = 0.0f;
    m_enemiesSpawnedInWave = 0;
    m_waveInProgress = false;
    m_gameOver = false;
    m_gameWin = false;
    m_gamePaused = false;
    m_elapsedTime = 0.0f;
    m_waveSpawnStates.clear();

    // 启动定时器
    m_lastTime = QDateTime::currentMSecsSinceEpoch();
    m_gameTimer->start();

    qDebug() << "GameManager: Level" << levelIndex << "initialized.";
    return true;
}

void GameManager::shutdownGame()
{
    m_gameTimer->stop();

    // 清理所有对象（先清理子弹、敌人、塔）
    // 敌人需要先清理其效果
    for (EnemyBase* enemy : m_enemies) {
        enemy->clearEffects();  // 删除效果
        delete enemy;
    }
    m_enemies.clear();

    for (TowerBase* tower : m_towers) {
        delete tower;
    }
    m_towers.clear();

    for (Bullet* bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();

    // 删除管理器
    delete m_gameMap;
    m_gameMap = nullptr;
    delete m_levelManager;
    m_levelManager = nullptr;

    m_resource = 0;
    m_currentWave = 0;
    m_gameOver = false;
    m_gameWin = false;

    m_waveSpawnStates.clear();
}

void GameManager::resetGame()
{
    if (!m_levelManager) {
        qWarning() << "GameManager: Cannot reset, no level loaded.";
        return;
    }
    int level = m_levelManager->getCurrentLevel();
    shutdownGame();
    initGame(level);
}

// ---------- 对象管理 ----------
void GameManager::addEnemy(EnemyBase* enemy)
{
    if (enemy) {
        m_enemies.append(enemy);
        emit enemyCountChanged(m_enemies.size());
    }
}

void GameManager::addTower(TowerBase* tower)
{
    if (tower) {
        m_towers.append(tower);
        emit towerCountChanged(m_towers.size());
    }
}

void GameManager::addBullet(Bullet* bullet)
{
    if (bullet) {
        m_bullets.append(bullet);
        bullet->setGameManager(this);
    }
}

void GameManager::markForRemoval(GameObject* obj)
{
    if (obj && obj->isActive()) {
        obj->setActive(false);
        // 注意：我们不在这里加入待删除列表，而是在cleanup中统一处理
        // 只需将isActive设为false即可
    }
}

// ---------- 游戏循环 ----------
void GameManager::onTimerTimeout()
{
    // 计算deltaTime
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastTime == 0) {
        m_lastTime = now;
        return;
    }
    m_deltaTime = (now - m_lastTime) / 1000.0f;
    m_lastTime = now;

    // 限制最大步长，防止卡顿时跳跃过大
    if (m_deltaTime > 0.1f) m_deltaTime = 0.1f;

    update(m_deltaTime);
}

void GameManager::update(float deltaTime)
{
    if (m_gamePaused || m_gameOver || m_gameWin) {
        return;
    }

    m_elapsedTime += deltaTime;   // 累加

    // 0. 构建敌人行分桶索引（塔的目标查找将使用此索引）
    buildEnemyRowIndex();

    // 1. 更新子弹（先设置敌人列表，以便碰撞检测）
    for (Bullet* bullet : m_bullets) {
        if (bullet->isActive()) {
            bullet->setEnemyList(&m_enemies);
            bullet->update(deltaTime);
        }
    }

    // 2. 更新塔（冷却缩减）
    for (TowerBase* tower : m_towers) {
        if (tower->isActive()) {
            tower->update(deltaTime);
        }
    }

    // 3. 更新敌人（移动、效果计时）
    QList<EnemyBase*> enemiesCopy = m_enemies;   // 复制副本
    for (EnemyBase* enemy : enemiesCopy) {
        if (!enemy->isActive()) continue;
        // 检测地形，设置速度倍率
        GridPos pos = enemy->getGridPos();
        float multiplier = 1.0f;
        if (pos.isValid() && m_gameMap->getTerrain(pos.row, pos.col) == TerrainType::ICE) {
            multiplier = 1.5f;
        }
        enemy->setTerrainSpeedMultiplier(multiplier);
        enemy->update(deltaTime);
    }

    // 4. 处理传送门（在敌人移动后立即检查）
    processTeleports();

    // 5. 波次生成
    updateWaveSpawning(deltaTime);

    // 6. 碰撞处理（敌人vs塔，子弹vs敌人）
    processCollisions();

    // 7. 检查胜负
    checkWinLoseConditions();

    // 8. 清理死亡对象
    cleanupDeadObjects();

    // 9. 通知UI刷新（单一定时器驱动，保证逻辑与渲染同步）
    emit gameUpdated();
}
/*
// ---------- 波次生成 ----------
void GameManager::updateWaveSpawning(float deltaTime)
{
    if (m_gameOver || m_gameWin) return;

    // 如果所有波次已完成，不再生成
    if (m_currentWave >= m_levelManager->getTotalWaves()) {
        return;
    }

    const WaveData& waveData = m_levelManager->getWave(m_currentWave);
    if (waveData.enemies.isEmpty()) {
        // 空波，直接进入下一波
        m_currentWave++;
        emit waveChanged(m_currentWave);
        return;
    }

    // 如果尚未开始该波，延迟
    if (!m_waveInProgress) {
        m_waveTimer += deltaTime;
        if (m_waveTimer >= waveData.waveDelay) {
            m_waveTimer = 0.0f;
            m_waveInProgress = true;
            m_enemiesSpawnedInWave = 0;
        }
        return;
    }

    // 波次进行中，按间隔生成敌人
    int totalEnemies = 0;
    for (const WaveEnemyInfo& info : waveData.enemies) {
        totalEnemies += info.count;
    }

    if (m_enemiesSpawnedInWave >= totalEnemies) {
        // 所有敌人生成完毕，该波结束
        m_waveInProgress = false;
        m_currentWave++;
        emit waveChanged(m_currentWave);
        return;
    }

    // 计算当前应生成的敌人类型（按顺序遍历）
    int accumulated = 0;
    for (const WaveEnemyInfo& info : waveData.enemies) {
        if (m_enemiesSpawnedInWave >= accumulated && m_enemiesSpawnedInWave < accumulated + info.count) {
            // 计时器
            m_waveTimer += deltaTime;
            if (m_waveTimer >= info.spawnInterval) {
                m_waveTimer = 0.0f;

                // ----- 修正：正确构造生成位置 -----
                WorldPos startPos;
                startPos.x = Constants::ENTRY_COL * Constants::CELL_SIZE + Constants::CELL_SIZE / 2.0f;
                startPos.y = info.spawnRow * Constants::CELL_SIZE + Constants::CELL_SIZE / 2.0f;

                EnemyBase* enemy = createEnemy(info.type, startPos);
                if (enemy) {
                    // 如果是召唤敌人，设置回调（可选）
                    if (info.type == EnemyType::SUMMON) {
                        EnemySummon* summon = dynamic_cast<EnemySummon*>(enemy);
                        if (summon) {
                            summon->setSpawnCallback([this](EnemyType type, const GridPos& gridPos) {
                                WorldPos pos = gridToWorld(gridPos);
                                EnemyBase* newEnemy = createEnemy(type, pos);
                                if (newEnemy) addEnemy(newEnemy);
                            });
                        }
                    }
                    addEnemy(enemy);
                }
                m_enemiesSpawnedInWave++;
            }
            break;
        }
        accumulated += info.count;
    }
}
*/

void GameManager::updateWaveSpawning(float deltaTime)
{
    if (m_gameOver || m_gameWin) return;
    if (m_currentWave >= m_levelManager->getTotalWaves()) return;

    const WaveData& waveData = m_levelManager->getWave(m_currentWave);
    if (waveData.enemies.isEmpty()) {
        m_currentWave++;
        emit waveChanged(m_currentWave);
        return;
    }

    // 如果尚未开始该波
    if (!m_waveInProgress) {
        m_waveTimer += deltaTime;
        if (m_waveTimer >= waveData.waveDelay) {
            m_waveTimer = 0.0f;
            m_waveInProgress = true;
            emit waveChanged(m_currentWave);
            // 初始化并行状态
            m_waveSpawnStates.clear();
            for (int i = 0; i < waveData.enemies.size(); ++i) {
                const WaveEnemyInfo& info = waveData.enemies[i];
                WaveSpawnState state;
                state.enemyIndex = i;
                state.remainingCount = info.count;
                state.timer = 0.0f;
                state.active = (info.count > 0);
                m_waveSpawnStates.append(state);
            }
        }
        return;
    }

    // 并行生成
    bool allFinished = true;
    for (auto& state : m_waveSpawnStates) {
        if (!state.active) continue;

        const WaveEnemyInfo& info = waveData.enemies[state.enemyIndex];
        state.timer += deltaTime;
        if (state.timer >= info.spawnInterval) {
            state.timer = 0.0f;
            // 生成敌人
            WorldPos startPos;
            startPos.x = Constants::ENTRY_COL * Constants::CELL_SIZE + Constants::CELL_SIZE / 2.0f;
            startPos.y = info.spawnRow * Constants::CELL_SIZE + Constants::CELL_SIZE / 2.0f;

            EnemyBase* enemy = createEnemy(info.type, startPos);
            if (enemy) {
                if (info.type == EnemyType::SUMMON) {
                    EnemySummon* summon = dynamic_cast<EnemySummon*>(enemy);
                    if (summon) {
                        summon->setSpawnCallback([this](EnemyType type, const GridPos& gridPos) {
                            WorldPos pos = gridToWorld(gridPos);
                            EnemyBase* newEnemy = createEnemy(type, pos);
                            if (newEnemy) addEnemy(newEnemy);
                        });
                    }
                }
                addEnemy(enemy);
            }
            state.remainingCount--;
            if (state.remainingCount <= 0) {
                state.active = false;
            }
        }

        // 处理之后仍活跃 → 波次尚未完成（避免同一帧完成时的延迟）
        if (state.active) {
            allFinished = false;
        }
    }

    if (allFinished) {
        m_waveInProgress = false;
        m_currentWave++;
        emit waveChanged(m_currentWave);
        m_waveSpawnStates.clear(); // 清理状态
    }
}

void GameManager::startNextWave()
{
    // 由外部调用，但此处用内部逻辑自动推进
}

bool GameManager::isAllWavesComplete() const
{
    return m_currentWave >= m_levelManager->getTotalWaves();
}
/*
void GameManager::processCollisions()
{
    // ---- 敌人 vs 防御塔 ----
    // 1. 先清除所有敌人的阻塞状态（假设本帧没有碰撞）
    for (EnemyBase* enemy : m_enemies) {
        if (enemy && enemy->isActive() && !enemy->isDead()) {
            enemy->setBlocked(false);
        }
    }

    // 2. 检测碰撞
    const float COLLIDE_THRESHOLD = 32.0f; // 可调整
    for (EnemyBase* enemy : m_enemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;

        for (TowerBase* tower : m_towers) {
            if (!tower || !tower->isActive() || tower->isDead()) continue;

            const WorldPos& ePos = enemy->getWorldPos();
            const WorldPos& tPos = tower->getWorldPos();
            float dx = ePos.x - tPos.x;
            float dy = ePos.y - tPos.y;
            float dist = std::sqrt(dx*dx + dy*dy);

            if (dist < COLLIDE_THRESHOLD) {
                // 塔受到敌人伤害
                tower->takeDamage(static_cast<float>(enemy->getDamage()));
                // 阻塞敌人（阻止其继续前进）
                enemy->setBlocked(true);
                // 可选：将敌人位置调整到塔的右侧边缘，防止重叠
                GridPos towerGrid = tower->getGridPos();
                WorldPos newPos = gridToWorld(towerGrid);
                newPos.x += Constants::CELL_SIZE / 2.0f; // 塔右侧边缘
                enemy->setWorldPos(newPos);
                // 每个敌人每帧只攻击一个塔
                break;
            }
        }
    }

    // ---- 子弹 vs 敌人 ----
    // 子弹的碰撞检测已在 Bullet::update 中通过 setEnemyList 和 checkCollisions 处理
    // 此处无需重复处理
}
*/
void GameManager::processCollisions()
{
    // ---- 1. 先清除所有敌人的阻塞状态 ----
    for (EnemyBase* enemy : m_enemies) {
        if (enemy && enemy->isActive() && !enemy->isDead()) {
            enemy->setBlocked(false);
        }
    }

    // ---- 2. 构建塔的空间索引（网格 → 塔列表），只需每帧一次 ----
    QVector<QVector<QList<TowerBase*>>> towerGrid(
        Constants::ROWS, QVector<QList<TowerBase*>>(Constants::COLS));
    for (TowerBase* tower : m_towers) {
        if (!tower || !tower->isActive() || tower->isDead()) continue;
        GridPos pos = tower->getGridPos();
        if (pos.isValid()) {
            towerGrid[pos.row][pos.col].append(tower);
        }
    }

    // ---- 3. 每个敌人只检查周边 3×3 格子内的塔 ----
    const float COLLIDE_THRESHOLD = 32.0f;
    const float COLLIDE_THRESHOLD_SQ = COLLIDE_THRESHOLD * COLLIDE_THRESHOLD;

    for (EnemyBase* enemy : m_enemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;

        GridPos eGrid = enemy->getGridPos();
        if (!eGrid.isValid()) continue;

        bool blocked = false;

        // 只检查敌人所在格及相邻 8 格（3×3 邻域）
        for (int dr = -1; dr <= 1 && !blocked; ++dr) {
            int r = eGrid.row + dr;
            if (r < 0 || r >= Constants::ROWS) continue;
            for (int dc = -1; dc <= 1 && !blocked; ++dc) {
                int c = eGrid.col + dc;
                if (c < 0 || c >= Constants::COLS) continue;

                for (TowerBase* tower : towerGrid[r][c]) {
                    const WorldPos& ePos = enemy->getWorldPos();
                    const WorldPos& tPos = tower->getWorldPos();
                    float dx = ePos.x - tPos.x;
                    float dy = ePos.y - tPos.y;
                    float distSq = dx * dx + dy * dy;

                    if (distSq < COLLIDE_THRESHOLD_SQ) {
                        blocked = true;

                        if (enemy->canAttack()) {
                            qDebug() << "Enemy" << enemy->getId() << "attacking tower" << tower->getId();
                            tower->takeDamage(static_cast<float>(enemy->getDamage()));
                            enemy->resetAttackCooldown();
                        }

                        // 将敌人位置调整到塔的右侧边缘（防止重叠）
                        WorldPos newPos = tower->getWorldPos();
                        newPos.x += Constants::CELL_SIZE / 2.0f + 2.0f;
                        enemy->setWorldPos(newPos);

                        break; // 每帧只攻击一个塔
                    }
                }
            }
        }

        enemy->setBlocked(blocked);
    }
}

// ---- 传送门处理 ----
void GameManager::processTeleports()
{
    if (!m_gameMap) return;

    for (EnemyBase* enemy : m_enemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;

        GridPos pos = enemy->getGridPos();
        if (!pos.isValid()) continue;

        // 检查是否已传送过该格子
        if (enemy->hasTeleportedGate(pos)) {
            continue;
        }

        if (m_gameMap->getTerrain(pos.row, pos.col) == TerrainType::TELEPORT) {
            GridPos dest = m_gameMap->getTeleportDestination(pos);
            if (dest != pos && dest.isValid()) {
                WorldPos newPos = gridToWorld(dest);
                enemy->setWorldPos(newPos);
                // 记录已传送的格子（入口和出口都记，防止再次触发）
                enemy->addTeleportedGate(pos);
                enemy->addTeleportedGate(dest);
                qDebug() << "Enemy" << enemy->getId() << "teleported from" << pos.row << pos.col << "to" << dest.row << dest.col;
            }
        }
    }
}

// ---- 检查胜负 ----
void GameManager::checkWinLoseConditions()
{
    // 失败：任何敌人到达左边缘（x <= 0）
    for (EnemyBase* enemy : m_enemies) {
        if (enemy->isActive() && !enemy->isDead()) {
            if (enemy->getWorldPos().x <= 0) {
                qDebug() << "Enemy" << enemy->getId() << "at x=" << enemy->getWorldPos().x << "triggered game over!";
                m_gameOver = true;
                m_gameTimer->stop();
                AudioManager::instance()->stopBackgroundMusic();
                emit gameOver();
                return;
            }
        }
    }
    // 胜利：所有波次完成，且没有敌人和子弹
    if (isAllWavesComplete() && m_enemies.isEmpty() && m_bullets.isEmpty()) {
        m_gameWin = true;
        m_gameTimer->stop();
        AudioManager::instance()->stopBackgroundMusic();
        emit gameWin();
    }
}

// ---- 清理死亡对象 ----
void GameManager::cleanupDeadObjects()
{
    // 清理敌人（先清除效果）
    for (auto it = m_enemies.begin(); it != m_enemies.end(); ) {
        EnemyBase* enemy = *it;
        if (!enemy->isActive() || enemy->isDead()) {
            enemy->clearEffects();  // 删除效果
            delete enemy;
            it = m_enemies.erase(it);
            emit enemyCountChanged(m_enemies.size());
        } else {
            ++it;
        }
    }

    // 清理塔（释放地图格子）
    for (auto it = m_towers.begin(); it != m_towers.end(); ) {
        TowerBase* tower = *it;
        if (!tower->isActive() || tower->isDead()) {
            // 释放地图格子
            GridPos pos = tower->getGridPos();
            m_gameMap->releaseGrid(pos.row, pos.col);
            delete tower;
            it = m_towers.erase(it);
            emit towerCountChanged(m_towers.size());
        } else {
            ++it;
        }
    }

    // 清理子弹
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
        Bullet* bullet = *it;
        if (!bullet->isActive() || bullet->isExpired()) {
            delete bullet;
            it = m_bullets.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------- 玩家操作 ----------
bool GameManager::buildTower(TowerType type, int row, int col)
{
    // 检查地形和占用
    if (!m_gameMap) {
        qWarning() << "GameManager: No map loaded.";
        return false;
    }
    if (!m_gameMap->isBuildable(row, col)) {
        qWarning() << "GameManager: Cannot build tower at (" << row << "," << col << ")";
        return false;
    }

    // 获取塔的基础费用
    int baseCost = ConfigLoader::getTowerCost(type);

    // 检查地形是否为黑土地（DARK_SOIL）
    TerrainType terrain = m_gameMap->getTerrain(row, col);
    int actualCost = baseCost;
    if (terrain == TerrainType::DARK_SOIL) {
        actualCost = static_cast<int>(baseCost * 0.7f);
        // 确保至少为1（如果原价很小）
        if (actualCost < 1) actualCost = 1;
    }

    if (m_resource < actualCost) {
        qWarning() << "GameManager: Insufficient resource. Need" << actualCost << ", have" << m_resource;
        return false;
    }

    // 扣除实际费用
    m_resource -= actualCost;
    emit resourceChanged(m_resource);

    // 创建塔对象
    WorldPos pos = gridToWorld(GridPos(row, col));
    TowerBase* tower = createTower(type, pos);
    if (!tower) {
        // 资源回退
        m_resource += actualCost;
        emit resourceChanged(m_resource);
        return false;
    }

    // 占用格子
    if (!m_gameMap->occupyGrid(row, col)) {
        // 理论上不会失败，因为已经检查过
        delete tower;
        m_resource += actualCost;
        emit resourceChanged(m_resource);
        return false;
    }

    // 添加到管理器
    addTower(tower);
    return true;
}

void GameManager::pauseGame()
{
    m_gamePaused = true;
}

void GameManager::resumeGame()
{
    m_gamePaused = false;
}

void GameManager::togglePause()
{
    m_gamePaused = !m_gamePaused;
}

void GameManager::cheatAddResource(int amount)
{
    m_resource += amount;
    emit resourceChanged(m_resource);
}

EnemyBase* GameManager::createEnemy(EnemyType type, const WorldPos& pos) {
    switch (type) {
    case EnemyType::NORMAL:
        return new EnemyNormal(pos);
    case EnemyType::FAST:
        return new EnemyFast(pos);
    case EnemyType::HEAVY:
        return new EnemyHeavy(pos);
    case EnemyType::RESISTANT:
        return new EnemyResistant(pos);
    case EnemyType::SUMMON:
        return new EnemySummon(pos);
    case EnemyType::BOSS:
        return new EnemyBoss(pos);
    default:
        qWarning() << "GameManager: Unknown enemy type, creating NORMAL.";
        return new EnemyNormal(pos);
    }
}

TowerBase* GameManager::createTower(TowerType type, const WorldPos& pos) {
    switch (type) {
    case TowerType::ARROW:
        return new TowerArrow(pos, this);
    case TowerType::SLOW:
        return new TowerSlow(pos, this);
    case TowerType::AOE:
        return new TowerAOE(pos, this);
    case TowerType::LASER:
        return new TowerLaser(pos, this);
    case TowerType::RESOURCE:
        return new TowerResource(pos, this);
    case TowerType::WALL:
        return new TowerWall(pos, this);
    default:
        qWarning() << "GameManager: Unknown tower type, creating ARROW.";
        return new TowerArrow(pos, this);
    }
}

void GameManager::addResource(int amount) {
    m_resource += amount;
    emit resourceChanged(m_resource);
}

// ---------- 行分桶索引 ----------

void GameManager::buildEnemyRowIndex()
{
    for (auto& list : m_enemiesByRow) {
        list.clear();
    }
    for (EnemyBase* enemy : m_enemies) {
        if (!enemy || !enemy->isActive() || enemy->isDead()) continue;
        int row = static_cast<int>(enemy->getWorldPos().y / Constants::CELL_SIZE);
        if (row >= 0 && row < Constants::ROWS) {
            m_enemiesByRow[row].append(enemy);
        }
    }
}

const QList<EnemyBase*>& GameManager::getEnemiesInRow(int row) const
{
    if (row >= 0 && row < m_enemiesByRow.size()) {
        return m_enemiesByRow[row];
    }
    static const QList<EnemyBase*> empty;
    return empty;
}

// ---------- 查询接口实现 ----------

TerrainType GameManager::getMapTerrain(int row, int col) const {
    if (!m_gameMap) {
        return TerrainType::GRASS;
    }
    return m_gameMap->getTerrain(row, col);
}

bool GameManager::canBuildTower(TowerType type, int row, int col) const {
    if (!m_gameMap) return false;
    // 检查地形和占用
    if (!m_gameMap->isBuildable(row, col)) return false;
    // 检查资源
    int cost = ConfigLoader::getTowerCost(type);
    if (m_resource < cost) return false;
    // 检查是否已有塔（isBuildable 已检查占用，但额外检查）
    return true;
}

} // namespace TowerDefense