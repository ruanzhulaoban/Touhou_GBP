#pragma once

#include <QString>

namespace TowerDefense {
namespace Constants {

constexpr int ROWS = 12;          // 网格行数
constexpr int COLS = 16;          // 网格列数
constexpr int CELL_SIZE = 64;     // 格子大小（像素）
constexpr int ENTRY_COL = 15;     // 敌人入口列（右侧边缘）
constexpr int EXIT_COL = 0;       // 敌人出口列（左侧边缘）

// 资源名称
inline QString resourceName() { return QStringLiteral("Chocolate Snail Bread"); }

} // namespace Constants
} // namespace TowerDefense