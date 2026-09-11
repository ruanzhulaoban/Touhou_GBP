#pragma once
#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QMenu>
#include "Enums.h"
#include "DataStructs.h"

namespace TowerDefense {
class GameManager;
class EnemyBase;
class TowerBase;
class Bullet;
}

namespace TowerDefenseUI {

struct SpriteAnimation {
    QVector<QPixmap> frames;
    float frameDuration;
    float currentTime;
    int currentFrame;
    SpriteAnimation() : frameDuration(0.1f), currentTime(0.0f), currentFrame(0) {}
};

class GameWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameWidget(TowerDefense::GameManager* manager, QWidget* parent = nullptr);
    ~GameWidget();

    void setSelectedTower(TowerDefense::TowerType type);
    void clearSelectedTower();
    void setPaused(bool paused);
signals:
    void towerSelectionCleared();   // 新增：当选择被清除时发射
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void loadAssets();
    void drawTerrain(QPainter& painter);
    void drawTowers(QPainter& painter);
    void drawEnemies(QPainter& painter);
    void drawBullets(QPainter& painter);
    void drawSelectedPreview(QPainter& painter);
    void drawPauseOverlay(QPainter& painter);
    void drawHealthBar(QPainter& painter, const QPointF& pos, float current, float max, int width, int height);

    QPixmap loadPixmap(const QString& path, const QSize& targetSize, const QColor& fallbackColor);

    TowerDefense::GameManager* m_gameManager;
    TowerDefense::TowerType m_selectedTower;
    bool m_hasSelectedTower;
    bool m_paused;

    // 缓存图片
    QMap<TowerDefense::TerrainType, QPixmap> m_terrainPixmaps;
    QMap<TowerDefense::TowerType, QPixmap> m_towerPixmaps;
    QMap<TowerDefense::EnemyType, QPixmap> m_enemyPixmaps;
    QPixmap m_bulletPixmap;
    QPixmap m_slowBulletPixmap;
    QPixmap m_laserBeamPixmap;
    QPixmap m_aoeRingPixmap;
    QPixmap m_starPixmap;

    // 鼠标悬停高亮
    TowerDefense::GridPos m_hoverGrid;
    bool m_hoverValid;

    QPixmap m_poisonIcon;
    QPixmap m_burnIcon;
    QPixmap m_slowIcon;
    // 在 private 成员中添加
    QPixmap m_teleportGatePixmap;   // 传送门门图片
    void showTowerMenu(TowerDefense::TowerBase* tower, const QPoint& globalPos);

};

} // namespace TowerDefenseUI