#pragma once

#include <QObject>
#include <QTimer>
#include <QList>
#include "Constants.h"
#include "Enums.h"
#include "DataStructs.h"
#include "GameObject.h"
#include "Enemy/EnemyBase.h"
#include "Tower/TowerBase.h"
#include "Bullet/Bullet.h"
#include "Map/GameMap.h"
#include "Wave_Level/LevelManager.h"

// 前向声明（具体塔和敌人类在cpp中包含）
class TowerArrow;
class TowerSlow;
class TowerAOE;
class TowerLaser;
class TowerResource;
class TowerWall;
class EnemyNormal;
class EnemyFast;
class EnemyHeavy;
class EnemyResistant;
class EnemySummon;
class EnemyBoss;

namespace TowerDefense {

/**
 * @brief 游戏总控与内存管理中心（单例模式，但为方便测试，采用普通类）
 * 
 * 继承QObject以使用信号槽，便于UI更新。
 */
class GameManager : public QObject
{
    Q_OBJECT

public:
    explicit GameManager(QObject *parent = nullptr);
    ~GameManager() override;

    // 禁止拷贝
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    // ---------- 初始化与生命周期 ----------
    /** 初始化指定关卡（加载地图、波次、设置初始资源） */
    bool initGame(int levelIndex);
    /** 清理所有对象，重置状态 */
    void shutdownGame();
    /** 重新开始当前关卡（需已经init） */
    void resetGame();

    // ---------- 对象管理（仅供内部和工厂使用） ----------
    void addEnemy(EnemyBase* enemy);
    void addTower(TowerBase* tower);
    void addBullet(Bullet* bullet);

    /** 标记对象死亡（由对象自身或碰撞逻辑调用） */
    void markForRemoval(GameObject* obj);

    // ---------- 游戏循环（由定时器驱动） ----------
    void onGameLoop();          // 定时器槽函数
    void update(float deltaTime);

    // ---------- 玩家操作接口 ----------
    /** 建造塔：检查地形、资源、占用，创建对应塔对象 */
    bool buildTower(TowerType type, int row, int col);
    void pauseGame();
    void resumeGame();
    void togglePause();
    void cheatAddResource(int amount);   // 作弊码

    // ---------- 查询接口（供UI显示） ----------
    int getResource() const { return m_resource; }
    int getCurrentWave() const { return m_currentWave; }
    int getEnemyCount() const { return m_enemies.size(); }
    int getTowerCount() const { return m_towers.size(); }
    bool isGameOver() const { return m_gameOver; }
    bool isGameWin() const { return m_gameWin; }
    bool isPaused() const { return m_gamePaused; }
    const QList<EnemyBase*>& getEnemies() const { return m_enemies; }

    // 行分桶索引：供塔的 findTargets() 使用，避免遍历全部敌人
    const QList<EnemyBase*>& getEnemiesInRow(int row) const;

    void addResource(int amount);

    // ----- 查询接口（供 UI 和 GameWidget 使用） -----
    TerrainType getMapTerrain(int row, int col) const;
    bool canBuildTower(TowerType type, int row, int col) const;
    int getTotalWaves() const { return m_levelManager ? m_levelManager->getTotalWaves() : 0; }

    // 获取游戏对象列表（返回 const 引用，避免拷贝）
    const QList<TowerBase*>& getTowers() const { return m_towers; }
    const QList<Bullet*>& getBullets() const { return m_bullets; }

    float getElapsedTime() const { return m_elapsedTime; }

    int getCurrentLevel() const { return m_levelManager ? m_levelManager->getCurrentLevel() : 0; }

signals:
    void gameOver();               // 失败
    void gameWin();                // 胜利
    void gameUpdated();            // 每帧逻辑更新完成后发射，驱动UI刷新
    void resourceChanged(int newResource);
    void waveChanged(int wave);
    void enemyCountChanged(int count);
    void towerCountChanged(int count);

private slots:
    void onTimerTimeout();         // 实际连接定时器的槽

private:
    // ---------- 内部更新子函数 ----------
    void updateWaveSpawning(float deltaTime);
    void startNextWave();
    bool isAllWavesComplete() const;

    /** 碰撞与交互逻辑（先敌人vs塔，后子弹vs敌人） */
    void processCollisions();

    /** 统一清理死亡对象（标记-清除） */
    void cleanupDeadObjects();

    /** 构建敌人行分桶索引（每帧在塔更新前调用一次） */
    void buildEnemyRowIndex();

    /** 处理传送门效果（敌人移动后调用） */
    void processTeleports();

    /** 检查胜负条件 */
    void checkWinLoseConditions();

    /** 创建具体敌人对象（由波次生成调用） */
    EnemyBase* createEnemy(EnemyType type, const WorldPos& pos);

    /** 创建具体塔对象（由buildTower调用） */
    TowerBase* createTower(TowerType type, const WorldPos& pos);

    // ---------- 数据成员 ----------
    // 游戏对象容器（原始指针）
    QList<EnemyBase*> m_enemies;
    QList<TowerBase*> m_towers;
    QList<Bullet*> m_bullets;

    // 行分桶索引（每帧重建）
    QVector<QList<EnemyBase*>> m_enemiesByRow;

    // 管理器（拥有所有权）
    GameMap* m_gameMap;
    LevelManager* m_levelManager;

    // 游戏状态
    int m_resource;                  // 当前资源（巧克力螺面包）
    int m_currentWave;               // 当前波次索引（0~4）
    float m_waveTimer;               // 波次生成计时器
    int m_enemiesSpawnedInWave;      // 当前波已生成数量
    bool m_waveInProgress;           // 是否正在生成敌人
    bool m_gamePaused;
    bool m_gameOver;
    bool m_gameWin;
    float m_elapsedTime;   // 游戏运行总时间（秒）

    // 循环控制
    QTimer* m_gameTimer;
    float m_deltaTime;               // 每帧时间增量（秒）
    qint64 m_lastTime;               // 用于计算deltaTime

    struct WaveSpawnState {
        int enemyIndex;
        int remainingCount;
        float timer;
        bool active;
    };
    QVector<WaveSpawnState> m_waveSpawnStates;
};

} // namespace TowerDefense