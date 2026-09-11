#pragma once

#include <QVector>
#include "WaveConfig.h"

namespace TowerDefense {

/**
 * @brief 关卡管理器
 * 
 * 负责加载当前关卡的地图、波次数据、初始资源等静态信息，
 * 并提供访问接口供 GameManager 使用。
 */
class LevelManager
{
public:
    LevelManager();
    ~LevelManager() = default;

    // 禁止拷贝
    LevelManager(const LevelManager&) = delete;
    LevelManager& operator=(const LevelManager&) = delete;

    /**
     * @brief 加载指定关卡
     * @param levelIndex 关卡索引（从0开始）
     * @return 加载成功返回 true
     */
    bool loadLevel(int levelIndex);

    // ----- 访问器 -----
    int getCurrentLevel() const { return m_currentLevel; }
    int getInitialResource() const { return m_initialResource; }
    int getDefaultSpawnRow() const { return m_defaultSpawnRow; }
    const QVector<WaveData>& getWaves() const { return m_waves; }

    /** 获取指定波次的数据（索引从0开始），若越界返回空 WaveData */
    WaveData getWave(int index) const;

    /** 获取总波数 */
    int getTotalWaves() const { return m_waves.size(); }

private:
    int m_currentLevel;          // 当前加载的关卡索引
    int m_initialResource;       // 初始巧克力螺面包数量
    int m_defaultSpawnRow;       // 默认敌人生成行（如果波次未指定）
    QVector<WaveData> m_waves;   // 本关卡所有波次数据
};

} // namespace TowerDefense