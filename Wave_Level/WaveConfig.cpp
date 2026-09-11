#include "WaveConfig.h"
#include "../ConfigLoader.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

namespace TowerDefense {

QVector<WaveData> WaveConfig::loadWaves(int levelIndex)
{
    QVector<WaveData> result;

    QJsonArray wavesArray = ConfigLoader::getLevelWaves(levelIndex);
    if (wavesArray.isEmpty()) {
        qWarning() << "WaveConfig: No waves found for level" << levelIndex;
        return result;
    }

    for (int i = 0; i < wavesArray.size(); ++i) {
        QJsonObject waveObj = wavesArray[i].toObject();
        WaveData wave;
        wave.waveIndex = i; // 使用数组索引作为波次编号（从0开始）

        // 读取 waveDelay（可选）
        wave.waveDelay = waveObj.value("waveDelay").toDouble(0.0);

        // 读取 enemies 数组
        QJsonArray enemiesArray = waveObj.value("enemies").toArray();
        for (int j = 0; j < enemiesArray.size(); ++j) {
            QJsonObject enemyObj = enemiesArray[j].toObject();
            WaveEnemyInfo info;

            QString typeStr = enemyObj.value("type").toString();
            info.type = stringToEnemyType(typeStr);
            info.count = enemyObj.value("count").toInt(1);
            info.spawnInterval = enemyObj.value("spawnInterval").toDouble(1.0);
            info.spawnRow = enemyObj.value("spawnRow").toInt(-1); // -1 表示未指定，由调用者填充

            wave.enemies.append(info);
        }

        result.append(wave);
    }

    return result;
}

EnemyType WaveConfig::stringToEnemyType(const QString& str)
{
    if (str == "NORMAL")   return EnemyType::NORMAL;
    if (str == "FAST")     return EnemyType::FAST;
    if (str == "HEAVY")    return EnemyType::HEAVY;
    if (str == "RESISTANT")return EnemyType::RESISTANT;
    if (str == "SUMMON")   return EnemyType::SUMMON;
    if (str == "BOSS")     return EnemyType::BOSS;
    qWarning() << "WaveConfig: Unknown enemy type string" << str << ", defaulting to NORMAL";
    return EnemyType::NORMAL;
}

} // namespace TowerDefense