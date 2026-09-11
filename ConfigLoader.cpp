#include "ConfigLoader.h"
#include "Constants.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

namespace TowerDefense {

// 静态成员初始化
QJsonObject ConfigLoader::m_root;
bool ConfigLoader::m_loaded = false;
QHash<TowerType, QJsonObject> ConfigLoader::s_towerCache;
QHash<EnemyType, QJsonObject> ConfigLoader::s_enemyCache;

// ---------- 公共接口 ----------

bool ConfigLoader::loadConfig(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ConfigLoader: Cannot open file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "ConfigLoader: JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "ConfigLoader: JSON root is not an object.";
        return false;
    }

    m_root = doc.object();
    m_loaded = true;
    buildCache();  // 一次性解析塔和敌人配置到缓存
    // 新增调试输出
    //qDebug() << "ConfigLoader: Loaded config, top-level keys:" << m_root.keys();
    //qDebug() << "ConfigLoader: Levels array size:" << m_root.value("levels").toArray().size();
    //qDebug() << "ConfigLoader: Towers object exists:" << m_root.contains("towers");
    //qDebug() << "ConfigLoader: Enemies object exists:" << m_root.contains("enemies");
    //qDebug() << "ConfigLoader: Configuration loaded successfully.";
    qDebug() << "=== ConfigLoader Debug ===";
    qDebug() << "Keys:" << m_root.keys();
    qDebug() << "Levels size:" << m_root.value("levels").toArray().size();
    return true;
}

bool ConfigLoader::isLoaded()
{
    return m_loaded && !m_root.isEmpty();
}

void ConfigLoader::buildCache()
{
    s_towerCache.clear();
    s_enemyCache.clear();

    // 缓存塔配置
    QJsonObject towers = m_root.value("towers").toObject();
    const TowerType towerTypes[] = {
        TowerType::ARROW, TowerType::SLOW, TowerType::AOE,
        TowerType::LASER, TowerType::RESOURCE, TowerType::WALL
    };
    for (TowerType t : towerTypes) {
        QString key = towerTypeToString(t);
        if (!key.isEmpty() && towers.contains(key)) {
            s_towerCache.insert(t, towers.value(key).toObject());
        }
    }

    // 缓存敌人配置
    QJsonObject enemies = m_root.value("enemies").toObject();
    const EnemyType enemyTypes[] = {
        EnemyType::NORMAL, EnemyType::FAST, EnemyType::HEAVY,
        EnemyType::RESISTANT, EnemyType::SUMMON, EnemyType::BOSS
    };
    for (EnemyType e : enemyTypes) {
        QString key = enemyTypeToString(e);
        if (!key.isEmpty() && enemies.contains(key)) {
            s_enemyCache.insert(e, enemies.value(key).toObject());
        }
    }
}

// ---------- 塔参数 ----------

int ConfigLoader::getTowerDamage(TowerType type)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return 10;
    return readInt(tower, "damage", 10);
}

double ConfigLoader::getTowerAttackInterval(TowerType type)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return 1.0;
    return readDouble(tower, "attackInterval", 1.0);
}

int ConfigLoader::getTowerCost(TowerType type)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return 50;
    return readInt(tower, "cost", 50);
}

double ConfigLoader::getTowerRange(TowerType type)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return 200.0;
    return readDouble(tower, "range", 200.0);
}

QJsonObject ConfigLoader::getTowerExtraParams(TowerType type)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return QJsonObject();
    // 返回除了通用字段（damage, attackInterval, cost, range）以外的所有字段
    QJsonObject extra;
    for (auto it = tower.begin(); it != tower.end(); ++it) {
        QString key = it.key();
        if (key != "damage" && key != "attackInterval" && key != "cost" && key != "range") {
            extra.insert(key, it.value());
        }
    }
    return extra;
}

// ---------- 敌人参数 ----------

int ConfigLoader::getEnemyHealth(EnemyType type)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return 100;
    return readInt(enemy, "health", 100);
}

double ConfigLoader::getEnemySpeed(EnemyType type)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return 50.0;
    return readDouble(enemy, "speed", 50.0);
}

int ConfigLoader::getEnemyDamage(EnemyType type)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return 10;
    return readInt(enemy, "damage", 10);
}

double ConfigLoader::getEnemySlowResistance(EnemyType type)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return 0.0;
    return readDouble(enemy, "slowResistance", 0.0);
}

// ---------- 波次配置 ----------

QJsonArray ConfigLoader::getWaveData(int waveIndex)
{
    QJsonArray waves = m_root.value("waves").toArray();
    if (waveIndex < 0 || waveIndex >= waves.size()) {
        qWarning() << "ConfigLoader: Wave index" << waveIndex << "out of range. Returning empty array.";
        return QJsonArray();
    }

    QJsonObject wave = waves[waveIndex].toObject();
    QJsonArray enemies = wave.value("enemies").toArray();
    return enemies;
}

// ---------- 关卡配置 ----------

int ConfigLoader::getLevelInitialResource(int levelIndex)
{
    QJsonArray levels = m_root.value("levels").toArray();
    if (levelIndex < 0 || levelIndex >= levels.size()) {
        qWarning() << "ConfigLoader: Level index" << levelIndex << "out of range. Returning default resource 200.";
        return 200;
    }

    QJsonObject level = levels[levelIndex].toObject();
    return readInt(level, "initialResource", 200);
}

TerrainType ConfigLoader::getMapTerrain(int levelIndex, int row, int col)
{
    // 检查行列范围
    if (row < 0 || row >= Constants::ROWS || col < 0 || col >= Constants::COLS) {
        qWarning() << "ConfigLoader: Map position (" << row << "," << col << ") out of range. Returning GRASS.";
        return TerrainType::GRASS;
    }

    QJsonArray levels = m_root.value("levels").toArray();
    if (levelIndex < 0 || levelIndex >= levels.size()) {
        qWarning() << "ConfigLoader: Level index" << levelIndex << "out of range. Returning GRASS.";
        return TerrainType::GRASS;
    }

    QJsonObject level = levels[levelIndex].toObject();
    QJsonArray mapData = level.value("map").toArray();

    // 检查行是否存在
    if (row >= mapData.size()) {
        qWarning() << "ConfigLoader: Map row" << row << "out of range. Returning GRASS.";
        return TerrainType::GRASS;
    }

    QJsonArray rowData = mapData[row].toArray();
    if (col >= rowData.size()) {
        qWarning() << "ConfigLoader: Map column" << col << "out of range. Returning GRASS.";
        return TerrainType::GRASS;
    }

    int terrainInt = rowData[col].toInt(0);
    return intToTerrainType(terrainInt);
}

// ---------- 私有辅助函数 ----------

QString ConfigLoader::towerTypeToString(TowerType type)
{
    switch (type) {
        case TowerType::ARROW:   return QStringLiteral("ARROW");
        case TowerType::SLOW:    return QStringLiteral("SLOW");
        case TowerType::AOE:     return QStringLiteral("AOE");
        case TowerType::LASER:   return QStringLiteral("LASER");
        case TowerType::RESOURCE:return QStringLiteral("RESOURCE");
        case TowerType::WALL:    return QStringLiteral("WALL");
        default: return QString();
    }
}

QString ConfigLoader::enemyTypeToString(EnemyType type)
{
    switch (type) {
        case EnemyType::NORMAL:   return QStringLiteral("NORMAL");
        case EnemyType::FAST:     return QStringLiteral("FAST");
        case EnemyType::HEAVY:    return QStringLiteral("HEAVY");
        case EnemyType::RESISTANT:return QStringLiteral("RESISTANT");
        case EnemyType::SUMMON:   return QStringLiteral("SUMMON");
        case EnemyType::BOSS:     return QStringLiteral("BOSS");
        default: return QString();
    }
}

TerrainType ConfigLoader::intToTerrainType(int val)
{
    switch (val) {
        case 0: return TerrainType::GRASS;
        case 1: return TerrainType::DARK_SOIL;
        case 2: return TerrainType::STONE;
        case 3: return TerrainType::ICE;
        case 4: return TerrainType::TELEPORT;
        default:
            qWarning() << "ConfigLoader: Unknown terrain int value" << val << ", defaulting to GRASS.";
            return TerrainType::GRASS;
    }
}

// ---------- 通用读取辅助 ----------

int ConfigLoader::readInt(const QJsonObject& obj, const QString& key, int defaultValue)
{
    if (obj.contains(key)) {
        return obj.value(key).toInt(defaultValue);
    } else {
        qWarning() << "ConfigLoader: Missing key" << key << ", using default" << defaultValue;
        return defaultValue;
    }
}

double ConfigLoader::readDouble(const QJsonObject& obj, const QString& key, double defaultValue)
{
    if (obj.contains(key)) {
        return obj.value(key).toDouble(defaultValue);
    } else {
        qWarning() << "ConfigLoader: Missing key" << key << ", using default" << defaultValue;
        return defaultValue;
    }
}

QString ConfigLoader::readString(const QJsonObject& obj, const QString& key, const QString& defaultValue)
{
    if (obj.contains(key)) {
        return obj.value(key).toString(defaultValue);
    } else {
        qWarning() << "ConfigLoader: Missing key" << key << ", using default" << defaultValue;
        return defaultValue;
    }
}

QJsonArray ConfigLoader::getLevelWaves(int levelIndex)
{
    QJsonArray levels = m_root.value("levels").toArray();
    if (levelIndex < 0 || levelIndex >= levels.size()) {
        qWarning() << "ConfigLoader: Level index" << levelIndex << "out of range for waves. Returning empty array.";
        return QJsonArray();
    }

    QJsonObject level = levels[levelIndex].toObject();
    QJsonArray waves = level.value("waves").toArray();
    return waves;
}

int ConfigLoader::getLevelDefaultSpawnRow(int levelIndex)
{
    QJsonArray levels = m_root.value("levels").toArray();
    if (levelIndex < 0 || levelIndex >= levels.size()) {
        qWarning() << "ConfigLoader: Level index" << levelIndex << "out of range for spawnRow. Returning default 6.";
        return 6; // 默认行（中间）
    }

    QJsonObject level = levels[levelIndex].toObject();
    return readInt(level, "defaultSpawnRow", 6);
}

double ConfigLoader::getEnemyExtraParam(EnemyType type, const QString& key, double defaultValue)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return defaultValue;
    return readDouble(enemy, key, defaultValue);
}

// ---------- 塔额外参数读取 ----------

double ConfigLoader::getTowerExtraParam(TowerType type, const QString& key, double defaultValue)
{
    const QJsonObject& tower = s_towerCache.value(type);
    if (tower.isEmpty()) return defaultValue;
    return readDouble(tower, key, defaultValue);
}

double ConfigLoader::getEnemyAttackInterval(EnemyType type)
{
    const QJsonObject& enemy = s_enemyCache.value(type);
    if (enemy.isEmpty()) return 1.0;
    return readDouble(enemy, "attackInterval", 1.0);
}

} // namespace TowerDefense

