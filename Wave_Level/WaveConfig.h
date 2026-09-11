#pragma once

#include <QVector>
#include "../Enums.h"

namespace TowerDefense {

/**
 * @brief 波次敌人信息（单种敌人）
 */
struct WaveEnemyInfo
{
    EnemyType type;          // 敌人类型
    int count;               // 数量
    float spawnInterval;     // 每个敌人之间的生成间隔（秒）
    int spawnRow;            // 生成行（默认可从关卡配置继承）
};

/**
 * @brief 单波数据
 */
struct WaveData
{
    int waveIndex;                      // 波次编号（从0或1开始，可自定义）
    QVector<WaveEnemyInfo> enemies;     // 本波包含的敌人序列（按生成顺序）
    float waveDelay;                    // 本波开始前的延迟（秒）
};

/**
 * @brief 波次配置管理器
 * 
 * 负责从 ConfigLoader 读取指定关卡的波次 JSON 数据，
 * 并解析为 WaveData 列表。
 */
class WaveConfig
{
public:
    /**
     * @brief 加载指定关卡的波次配置
     * @param levelIndex 关卡索引（从0开始）
     * @return 包含所有波次的 QVector<WaveData>
     */
    static QVector<WaveData> loadWaves(int levelIndex);

private:
    /** 将字符串转换为 EnemyType */
    static EnemyType stringToEnemyType(const QString& str);
};

} // namespace TowerDefense