#pragma once

#include <QtGlobal>
#include <cmath>
#include "Constants.h"

namespace TowerDefense {

// 网格坐标（行、列）
struct GridPos {
    int row;
    int col;

    GridPos(int r = 0, int c = 0) : row(r), col(c) {}

    bool operator==(const GridPos& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const GridPos& other) const {
        return !(*this == other);
    }

    bool isValid() const {
        return row >= 0 && row < Constants::ROWS &&
               col >= 0 && col < Constants::COLS;
    }
};

// 哈希支持（用于 QHash、QSet 等）
inline uint qHash(const GridPos& key, uint seed = 0) {
    return (static_cast<uint>(key.row) << 16) ^ static_cast<uint>(key.col) ^ seed;
}

// 世界坐标（像素）
struct WorldPos {
    double x;
    double y;

    WorldPos(double x_ = 0.0, double y_ = 0.0) : x(x_), y(y_) {}

    bool operator==(const WorldPos& other) const {
        return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y);
    }
};

// 将网格坐标转换为世界坐标（格子中心点）
inline WorldPos gridToWorld(const GridPos& grid, int cellSize = Constants::CELL_SIZE) {
    double x = grid.col * cellSize + cellSize / 2.0;
    double y = grid.row * cellSize + cellSize / 2.0;
    return WorldPos(x, y);
}

// 将世界坐标转换为网格坐标，如果超出有效范围则返回无效 GridPos，
// 通过 ok 指针返回有效性（可选）
inline GridPos worldToGrid(const WorldPos& world, int cellSize = Constants::CELL_SIZE, bool* ok = nullptr) {
    int col = static_cast<int>(std::floor(world.x / cellSize));
    int row = static_cast<int>(std::floor(world.y / cellSize));
    GridPos pos(row, col);
    bool valid = pos.isValid();
    if (ok) *ok = valid;
    return pos;
}

} // namespace TowerDefense