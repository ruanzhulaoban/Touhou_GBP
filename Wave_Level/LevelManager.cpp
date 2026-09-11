#include "LevelManager.h"
#include "../ConfigLoader.h"
#include "../Constants.h"
#include <QDebug>

namespace TowerDefense {

LevelManager::LevelManager()
    : m_currentLevel(-1)
    , m_initialResource(0)
    , m_defaultSpawnRow(6)
{
}

bool LevelManager::loadLevel(int levelIndex)
{
    // 1. 读取初始资源
    m_initialResource = ConfigLoader::getLevelInitialResource(levelIndex);

    // 2. 读取默认生成行（如果关卡配置中有）
    m_defaultSpawnRow = ConfigLoader::getLevelDefaultSpawnRow(levelIndex);

    // 3. 加载波次数据
    m_waves = WaveConfig::loadWaves(levelIndex);
    if (m_waves.isEmpty()) {
        qWarning() << "LevelManager: No waves loaded for level" << levelIndex;
        // 即使没有波次，也继续（但游戏将无法进行）
    }

    // 4. 补全每个波次中敌人的生成行（若未指定）
    for (auto& wave : m_waves) {
        for (auto& enemyInfo : wave.enemies) {
            if (enemyInfo.spawnRow < 0 || enemyInfo.spawnRow >= Constants::ROWS) {
                enemyInfo.spawnRow = m_defaultSpawnRow;
            }
        }
    }

    m_currentLevel = levelIndex;
    qDebug() << "LevelManager: Loaded level" << levelIndex
             << "with" << m_waves.size() << "waves, initial resource" << m_initialResource
             << ", default spawn row" << m_defaultSpawnRow;
    return true;
}

WaveData LevelManager::getWave(int index) const
{
    if (index >= 0 && index < m_waves.size()) {
        return m_waves[index];
    }
    return WaveData(); // 返回空数据
}

} // namespace TowerDefense