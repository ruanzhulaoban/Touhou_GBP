#pragma once

#include <QString>

namespace TowerDefense {

enum class TerrainType {
    GRASS,       // 普通草地
    DARK_SOIL,   // 黑土地
    STONE,       // 石地
    ICE,         // 冰面
    TELEPORT     // 传送门
};

enum class TowerType {
    ARROW,       // 普通射手塔
    SLOW,        // 减速塔
    AOE,         // 范围攻击塔
    LASER,       // 激光塔
    RESOURCE,    // 资源塔
    WALL         // 防御墙
};

enum class EnemyType {
    NORMAL,      // 普通敌人
    FAST,        // 快速敌人
    HEAVY,       // 重甲敌人
    RESISTANT,   // 抗性敌人
    SUMMON,      // 召唤敌人
    BOSS         // Boss敌人
};

enum class EffectType {
    NONE,   // 无特殊效果
    SLOW,        // 减速
    POISON,      // 中毒
    BURN         // 灼烧
};

} // namespace TowerDefense