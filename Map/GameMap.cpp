#include "GameMap.h"
#include "../ConfigLoader.h"
#include <algorithm>
#include <QDebug>

namespace TowerDefense {

GameMap::GameMap()
    : m_rows(Constants::ROWS)
    , m_cols(Constants::COLS)
{
    // 初始化地形网格（默认草地）
    m_terrainGrid.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_terrainGrid[r].resize(m_cols, TerrainType::GRASS);
    }

    // 初始化占用网格（全 false）
    m_occupiedGrid.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_occupiedGrid[r].resize(m_cols, false);
    }
}

bool GameMap::loadMap(int levelIndex)
{
    // 1. 填充地形
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_terrainGrid[r][c] = ConfigLoader::getMapTerrain(levelIndex, r, c);
        }
    }

    // 2. 重置占用网格
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_occupiedGrid[r][c] = false;
        }
    }

    // 3. 清空旧的传送门配对并重新构建
    m_teleportPairs.clear();
    buildTeleportPairs();

    return true;
}

TerrainType GameMap::getTerrain(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return TerrainType::GRASS;
    }
    return m_terrainGrid[row][col];
}

bool GameMap::isBuildable(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return false;
    }
    TerrainType terrain = m_terrainGrid[row][col];
    if (terrain == TerrainType::STONE || terrain == TerrainType::TELEPORT) {
        return false;
    }
    if (m_occupiedGrid[row][col]) {
        return false;
    }
    return true;
}

bool GameMap::occupyGrid(int row, int col)
{
    if (!isBuildable(row, col)) {
        return false;
    }
    m_occupiedGrid[row][col] = true;
    return true;
}

void GameMap::releaseGrid(int row, int col)
{
    if (row >= 0 && row < m_rows && col >= 0 && col < m_cols) {
        m_occupiedGrid[row][col] = false;
    }
}

GridPos GameMap::getTeleportDestination(const GridPos& pos) const
{
    if (m_teleportPairs.contains(pos)) {
        return m_teleportPairs[pos];
    }
    return pos; // 不是传送门入口或未配对，返回原位置
}

void GameMap::buildTeleportPairs()
{
    // 收集所有传送门格子
    QList<GridPos> teleportGates;
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            if (m_terrainGrid[r][c] == TerrainType::TELEPORT) {
                teleportGates.append(GridPos(r, c));
            }
        }
    }

    // 按行主序排序（先比较行，再比较列）
    std::sort(teleportGates.begin(), teleportGates.end(),
              [](const GridPos& a, const GridPos& b) {
                  if (a.row != b.row) return a.row < b.row;
                  return a.col < b.col;
              });

    // 两两配对
    for (int i = 0; i + 1 < teleportGates.size(); i += 2) {
        const GridPos& in = teleportGates[i];
        const GridPos& out = teleportGates[i + 1];
        m_teleportPairs[in] = out;
        m_teleportPairs[out] = in;
    }

    // 如果数量为奇数，最后一个不配对（无操作）
}

} // namespace TowerDefense