#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include "Enums.h"

namespace TowerDefense {

/**
 * @brief 配置文件加载器（单例风格，所有方法静态）
 * 
 * 负责加载 JSON 配置文件，并提供统一的查询接口。
 * 若字段缺失，返回默认值并输出 qWarning 警告。
 */
class ConfigLoader
{
public:
    /**
     * @brief 加载配置文件
     * @param filePath 配置文件路径（例如 "./config/game_config.json"）
     * @return 是否加载成功
     */
    static bool loadConfig(const QString& filePath);

    /** @brief 是否已成功加载配置 */
    static bool isLoaded();

    // ----- 塔参数 -----
    static int getTowerDamage(TowerType type);
    static double getTowerAttackInterval(TowerType type);
    static int getTowerCost(TowerType type);
    static double getTowerRange(TowerType type);

    /**
     * @brief 获取塔的额外参数（如减速比例、AOE半径等）
     * @return 包含额外参数的 QJsonObject，若不存在则返回空对象
     */
    static QJsonObject getTowerExtraParams(TowerType type);

    // ----- 敌人参数 -----
    static int getEnemyHealth(EnemyType type);
    static double getEnemySpeed(EnemyType type);
    static int getEnemyDamage(EnemyType type);
    static double getEnemySlowResistance(EnemyType type);
    static double getEnemyAttackInterval(EnemyType type);

    // ----- 波次配置 -----
    /**
     * @brief 获取第 waveIndex 波的数据
     * @param waveIndex 波次索引（从 0 开始）
     * @return 包含该波敌人列表的 QJsonArray，每个元素为 { "type": "...", "count": int, "spawnInterval": double(ms) }
     */
    static QJsonArray getWaveData(int waveIndex);

    // ----- 关卡配置 -----
    static int getLevelInitialResource(int levelIndex);

    /**
     * @brief 获取特定关卡指定格子的地形
     * @param levelIndex 关卡索引（从 0 开始）
     * @param row 行
     * @param col 列
     * @return 地形类型，若超出范围或数据缺失返回 TerrainType::GRASS
     */
    static TerrainType getMapTerrain(int levelIndex, int row, int col);

    // ----- 关卡波次与生成行（第三阶段新增） -----
    static QJsonArray getLevelWaves(int levelIndex);
    static int getLevelDefaultSpawnRow(int levelIndex);

    // ----- 敌人额外参数（第五阶段新增） -----
    static double getEnemyExtraParam(EnemyType type, const QString& key, double defaultValue);

    static double getTowerExtraParam(TowerType type, const QString& key, double defaultValue);
private:
    static QJsonObject m_root;      // 加载的根 JSON 对象
    static bool m_loaded;           // 加载成功标志

    // 缓存：loadConfig 时一次性解析，避免每次访问都遍历 JSON 树
    static QHash<TowerType, QJsonObject> s_towerCache;
    static QHash<EnemyType, QJsonObject> s_enemyCache;
    static void buildCache();       // 填充缓存

    // 枚举与字符串的转换辅助函数
    static QString towerTypeToString(TowerType type);
    static QString enemyTypeToString(EnemyType type);
    static TerrainType intToTerrainType(int val);

    // 通用读取辅助（返回默认值）
    static int readInt(const QJsonObject& obj, const QString& key, int defaultValue);
    static double readDouble(const QJsonObject& obj, const QString& key, double defaultValue);
    static QString readString(const QJsonObject& obj, const QString& key, const QString& defaultValue);
};

} // namespace TowerDefense