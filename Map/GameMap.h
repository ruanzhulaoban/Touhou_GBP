#pragma once

#include <QVector>
#include <QHash>
#include "../Constants.h"
#include "../Enums.h"
#include "../DataStructs.h"

namespace TowerDefense {

/**
 * @brief 地图管理器
 * 
 * 管理 12x16 的地形网格、塔占用状态以及传送门配对。
 * 不涉及任何游戏对象，只处理静态地形数据。
 */
class GameMap
{
public:
    GameMap();
    ~GameMap() = default;

    // 禁止拷贝
    GameMap(const GameMap&) = delete;
    GameMap& operator=(const GameMap&) = delete;

    /**
     * @brief 从 ConfigLoader 加载第 levelIndex 关的地图数据
     * @param levelIndex 关卡索引（从0开始）
     * @return 加载成功返回 true
     */
    bool loadMap(int levelIndex);

    /** 获取指定格子的地形类型（越界返回 GRASS） */
    TerrainType getTerrain(int row, int col) const;

    /** 判断格子是否可建造（非石地、未被占用、在地图内） */
    bool isBuildable(int row, int col) const;

    /**
     * @brief 尝试占用格子（建造塔时调用）
     * @return 成功占用返回 true，失败（不可建造或已被占用）返回 false
     */
    bool occupyGrid(int row, int col);

    /** 释放格子占用（塔被摧毁时调用） */
    void releaseGrid(int row, int col);

    /**
     * @brief 获取传送门出口
     * @param pos 当前位置
     * @return 如果该格子是传送门入口，返回配对的出口；否则返回原格子
     */
    GridPos getTeleportDestination(const GridPos& pos) const;

    // 获取网格尺寸
    int getRows() const { return m_rows; }
    int getCols() const { return m_cols; }

private:
    int m_rows;
    int m_cols;
    QVector<QVector<TerrainType>> m_terrainGrid;   // 12x16 地形
    QVector<QVector<bool>> m_occupiedGrid;         // 12x16 占用标记
    QHash<GridPos, GridPos> m_teleportPairs;       // 传送门配对表

    /** 构建传送门配对（按行主序两两配对） */
    void buildTeleportPairs();
};

} // namespace TowerDefense