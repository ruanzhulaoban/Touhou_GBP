#include "GameWidget.h"
#include "GameManager.h"
#include "ConfigLoader.h"
#include "Constants.h"
#include "Bullet/SlowBullet.h"
#include "Tower/TowerLaser.h"
#include "Tower/TowerAOE.h"
#include "Tower/TowerBase.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QFont>
#include <QMainWindow>
#include <QStatusBar>
#include <cmath>
#include <QMenu>
#include <QAction>

namespace TowerDefenseUI {

GameWidget::GameWidget(TowerDefense::GameManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_gameManager(manager)
    , m_selectedTower(TowerDefense::TowerType::ARROW)
    , m_hasSelectedTower(false)
    , m_paused(false)
    , m_hoverValid(false)
{
    setMinimumSize(TowerDefense::Constants::COLS * TowerDefense::Constants::CELL_SIZE,
                   TowerDefense::Constants::ROWS * TowerDefense::Constants::CELL_SIZE);
    setMouseTracking(true); // 启用鼠标移动跟踪

    loadAssets();
}

GameWidget::~GameWidget() {
}

void GameWidget::loadAssets() {
    // 加载地形
    QSize cellSize(TowerDefense::Constants::CELL_SIZE, TowerDefense::Constants::CELL_SIZE);
    m_terrainPixmaps[TowerDefense::TerrainType::GRASS] = loadPixmap("./assets/images/grass.png", cellSize, Qt::green);
    m_terrainPixmaps[TowerDefense::TerrainType::DARK_SOIL] = loadPixmap("./assets/images/dark_soil.png", cellSize, Qt::darkGray);
    m_terrainPixmaps[TowerDefense::TerrainType::STONE] = loadPixmap("./assets/images/stone.png", cellSize, Qt::gray);
    m_terrainPixmaps[TowerDefense::TerrainType::ICE] = loadPixmap("./assets/images/ice.png", cellSize, Qt::cyan);
    m_terrainPixmaps[TowerDefense::TerrainType::TELEPORT] = loadPixmap("./assets/images/grass.png", cellSize, Qt::magenta);

    // 加载塔
    int towerSize = static_cast<int>(TowerDefense::Constants::CELL_SIZE * 0.95);
    QSize tSize(towerSize, towerSize);
    m_towerPixmaps[TowerDefense::TowerType::ARROW] = loadPixmap("./assets/images/tower_arrow.png", tSize, Qt::red);
    m_towerPixmaps[TowerDefense::TowerType::SLOW] = loadPixmap("./assets/images/tower_slow.png", tSize, Qt::blue);
    m_towerPixmaps[TowerDefense::TowerType::AOE] = loadPixmap("./assets/images/tower_aoe.png", tSize, Qt::darkRed);
    m_towerPixmaps[TowerDefense::TowerType::LASER] = loadPixmap("./assets/images/tower_laser.png", tSize, Qt::yellow);
    m_towerPixmaps[TowerDefense::TowerType::RESOURCE] = loadPixmap("./assets/images/tower_resource.png", tSize, Qt::green);
    m_towerPixmaps[TowerDefense::TowerType::WALL] = loadPixmap("./assets/images/tower_wall.png", tSize, Qt::darkYellow);

    // 加载敌人
    int enemySize = static_cast<int>(TowerDefense::Constants::CELL_SIZE * 0.9);
    QSize eSize(enemySize, enemySize);
    m_enemyPixmaps[TowerDefense::EnemyType::NORMAL] = loadPixmap("./assets/images/enemy_normal.png", eSize, Qt::lightGray);
    m_enemyPixmaps[TowerDefense::EnemyType::FAST] = loadPixmap("./assets/images/enemy_fast.png", eSize, Qt::yellow);
    m_enemyPixmaps[TowerDefense::EnemyType::HEAVY] = loadPixmap("./assets/images/enemy_heavy.png", eSize, Qt::darkGray);
    m_enemyPixmaps[TowerDefense::EnemyType::RESISTANT] = loadPixmap("./assets/images/enemy_resistant.png", eSize, Qt::cyan);
    m_enemyPixmaps[TowerDefense::EnemyType::SUMMON] = loadPixmap("./assets/images/enemy_summon.png", eSize, Qt::magenta);
    m_enemyPixmaps[TowerDefense::EnemyType::BOSS] = loadPixmap("./assets/images/enemy_boss.png", eSize, Qt::red);

    // 加载子弹
    int bulletSize = 16;
    QSize bSize(bulletSize, bulletSize);
    m_bulletPixmap = loadPixmap("./assets/images/bullet.png", bSize, Qt::white);
    // 加载减速子弹图片（若不存在则使用普通子弹的备用色）
    m_slowBulletPixmap = loadPixmap("./assets/images/slow_bullet.png", bSize, QColor(0, 150, 255));
    // 加载激光光束（尺寸临时设 200x20，实际会拉伸）
    QSize beamSize(200, 20);
    m_laserBeamPixmap = loadPixmap("./assets/images/laser_beam.png", beamSize, QColor(255, 50, 50, 180));
    // 加载AOE环形法阵（尺寸设为 200x200，实际会缩放）
    QSize ringSize(200, 200);
    m_aoeRingPixmap = loadPixmap("./assets/images/aoe_ring.png", ringSize, QColor(255, 200, 100, 150));

    // 加载状态图标（预缩放到绘制尺寸 ICON_SIZE=12，避免每帧缩放）
    QSize iconSize(12, 12);
    m_poisonIcon = loadPixmap("./assets/images/poison_icon.png", iconSize, QColor(128, 0, 128));
    m_burnIcon   = loadPixmap("./assets/images/burn_icon.png",   iconSize, QColor(255, 100, 0));
    m_slowIcon   = loadPixmap("./assets/images/slow_icon.png",   iconSize, QColor(0, 200, 255));

    // 加载传送门门图片（尺寸为格子大小）
    QSize gateSize(TowerDefense::Constants::CELL_SIZE, TowerDefense::Constants::CELL_SIZE);
    m_teleportGatePixmap = loadPixmap("./assets/images/teleport_gate.png", gateSize, QColor(255, 0, 255, 100));

    // 加载星星图片（预缩放到绘制尺寸 STAR_SIZE=24，避免每帧缩放）
    QSize starSize(24, 24);
    m_starPixmap = loadPixmap("./assets/images/star.png", starSize, QColor(255, 215, 0));
}

QPixmap GameWidget::loadPixmap(const QString& path, const QSize& targetSize, const QColor& fallbackColor) {
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        qWarning() << "GameWidget: Failed to load" << path << ", using fallback color.";
        pixmap = QPixmap(targetSize);
        pixmap.fill(fallbackColor);
        // 绘制边框以区分
        QPainter p(&pixmap);
        p.setPen(Qt::black);
        p.drawRect(0, 0, targetSize.width()-1, targetSize.height()-1);
        p.end();
    } else {
        pixmap = pixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return pixmap;
}

void GameWidget::setSelectedTower(TowerDefense::TowerType type) {
    m_selectedTower = type;
    m_hasSelectedTower = true;
    update();
}

void GameWidget::clearSelectedTower() {
    m_hasSelectedTower = false;
    emit towerSelectionCleared();   // 新增
    update();
}

void GameWidget::setPaused(bool paused) {
    m_paused = paused;
    update();
}

void GameWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    drawTerrain(painter);
    drawTowers(painter);
    drawEnemies(painter);
    drawBullets(painter);
    drawSelectedPreview(painter);
    if (m_paused) {
        drawPauseOverlay(painter);
    }
}

void GameWidget::drawTerrain(QPainter& painter) {
    int rows = TowerDefense::Constants::ROWS;
    int cols = TowerDefense::Constants::COLS;
    int cellSize = TowerDefense::Constants::CELL_SIZE;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            TowerDefense::TerrainType terrain = m_gameManager->getMapTerrain(r, c);
            QPixmap pix = m_terrainPixmaps.value(terrain);
            if (pix.isNull()) {
                painter.fillRect(c*cellSize, r*cellSize, cellSize, cellSize, Qt::lightGray);
            } else {
                painter.drawPixmap(c*cellSize, r*cellSize, cellSize, cellSize, pix);
            }

            // 2. 如果是传送门，叠加门图片
            if (terrain == TowerDefense::TerrainType::TELEPORT) {
                if (!m_teleportGatePixmap.isNull()) {
                    painter.drawPixmap(c*cellSize, r*cellSize, cellSize, cellSize, m_teleportGatePixmap);
                } else {
                    // 备用：绘制半透明紫色门标志
                    painter.fillRect(c*cellSize + 8, r*cellSize + 8, cellSize - 16, cellSize - 16, QColor(255, 0, 255, 100));
                }
            }

        }
    }
}

void GameWidget::drawTowers(QPainter& painter) {
    const QList<TowerDefense::TowerBase*>& towers = m_gameManager->getTowers();
    int cellSize = TowerDefense::Constants::CELL_SIZE;

    for (TowerDefense::TowerBase* tower : towers) {
        if (!tower->isActive()) continue;
        const TowerDefense::WorldPos& pos = tower->getWorldPos();
        QPointF drawPos(pos.x - cellSize*0.475, pos.y - cellSize*0.475);
        QPixmap pix = m_towerPixmaps.value(tower->getType());
        painter.drawPixmap(drawPos, pix);

        drawHealthBar(painter, QPointF(pos.x, pos.y - cellSize*0.45),
                      tower->getHealth(), tower->getMaxHealth(), 30, 4);

        // ---- 绘制星星表示等级（竖向排布） ----
        int level = tower->getUpgradeLevel();
        if (level >= 2) {
            int starCount = level - 1;   // 2级1星，3级2星
            const int STAR_SIZE = 24;    // 调大星星
            const int SPACING = 28;      // 垂直间距
            // 塔左侧居中：X偏移为 -0.7倍格子，Y方向垂直居中
            float startX = pos.x - cellSize * 0.4 - STAR_SIZE / 2.0f;
            float totalHeight = (starCount - 1) * SPACING + STAR_SIZE;
            float startY = pos.y - totalHeight / 2.0f;
            for (int i = 0; i < starCount; ++i) {
                QPointF starPos(startX, startY + i * SPACING);
                if (!m_starPixmap.isNull()) {
                    painter.drawPixmap(starPos, m_starPixmap);
                } else {
                    // 备用：绘制金色圆点
                    painter.setBrush(QColor(255, 215, 0));
                    painter.setPen(Qt::NoPen);
                    painter.drawEllipse(starPos, STAR_SIZE/2, STAR_SIZE/2);
                }
            }
        }

        // ---- AOE 环形法阵动画 ----
        if (tower->getType() == TowerDefense::TowerType::AOE) {
            TowerDefense::TowerAOE* aoe = dynamic_cast<TowerDefense::TowerAOE*>(tower);
            if (aoe && aoe->getRingTimer() > 0.0f) {
                float progress = 1.0f - aoe->getRingTimer() / aoe->getRingDuration(); // 0→1
                float maxRadius = aoe->getRange();
                float currentRadius = maxRadius * progress; // 从0扩大到射程
                float alpha = 1.0f - progress * 0.8f; // 逐渐变淡（开始不透明，结束半透明）

                QPixmap ring = m_aoeRingPixmap;
                if (!ring.isNull()) {
                    // 缩放至当前半径（2倍半径作为正方形尺寸）
                    int size = static_cast<int>(currentRadius * 2);
                    if (size < 4) size = 4; // 防太小
                    QPixmap scaledRing = ring.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    // 设置透明度
                    QPainter p(&scaledRing);
                    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                    p.fillRect(scaledRing.rect(), QColor(255, 255, 255, static_cast<int>(alpha * 255)));
                    p.end();
                    // 绘制在塔中心（偏移使居中）
                    painter.drawPixmap(QPointF(pos.x - size/2.0f, pos.y - size/2.0f), scaledRing);
                } else {
                    // 备用：绘制半透明圆形边框
                    painter.setPen(QPen(QColor(255, 200, 100, static_cast<int>(alpha * 200)), 3));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawEllipse(QPointF(pos.x, pos.y), currentRadius, currentRadius);
                }
            }
        }

        // ---- 激光塔光束动画 ----
        if (tower->getType() == TowerDefense::TowerType::LASER) {
            TowerDefense::TowerLaser* laser = dynamic_cast<TowerDefense::TowerLaser*>(tower);
            if (laser && laser->getLaserTimer() > 0.0f) {
                float maxBeamX = TowerDefense::Constants::COLS * TowerDefense::Constants::CELL_SIZE; // 地图右边界（像素）
                float maxLength = maxBeamX - pos.x;   // 塔到地图右边的距离
                float beamLength = qMin(laser->getRange(), maxLength);
                if (beamLength <= 0) break; // 如果塔已超出地图（不应发生），跳过绘制
                float alpha = qMin(1.0f, laser->getLaserTimer() * 5.0f);
                QPixmap beam = m_laserBeamPixmap;
                const int BEAM_HEIGHT = 8;   // 调整这里的值控制粗细
                if (!beam.isNull()) {
                    QSize targetSize(static_cast<int>(beamLength), BEAM_HEIGHT);
                    QPixmap scaledBeam = beam.scaled(targetSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    // 透明度处理（如果有）
                    QPainter p(&scaledBeam);
                    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                    p.fillRect(scaledBeam.rect(), QColor(255, 255, 255, static_cast<int>(alpha * 255)));
                    p.end();
                    painter.drawPixmap(QPointF(pos.x, pos.y - BEAM_HEIGHT/2.0f), scaledBeam);
                } else {
                    painter.fillRect(QRectF(pos.x, pos.y - BEAM_HEIGHT/2.0f, beamLength, BEAM_HEIGHT), QColor(255, 0, 0, 150));
                }
            }
        }
    }
}

void GameWidget::drawEnemies(QPainter& painter) {
    const QList<TowerDefense::EnemyBase*>& enemies = m_gameManager->getEnemies();
    int cellSize = TowerDefense::Constants::CELL_SIZE;

    for (TowerDefense::EnemyBase* enemy : enemies) {
        if (!enemy->isActive() || enemy->isDead()) continue;
        const TowerDefense::WorldPos& pos = enemy->getWorldPos();

        // 1. 绘制敌人图片
        QPixmap pix = m_enemyPixmaps.value(enemy->getType());
        QPointF drawPos(pos.x - cellSize * 0.45, pos.y - cellSize * 0.45);
        painter.drawPixmap(drawPos, pix);

        // 2. 绘制血条（在敌人上方）
        drawHealthBar(painter, QPointF(pos.x, pos.y - cellSize * 0.25),
                      enemy->getHealth(), enemy->getMaxHealth(), 30, 4);

        // 3. 绘制状态图标 + 环形进度条（在血条上方）
        QPointF iconBase(pos.x - 30, pos.y - cellSize * 0.65); // 起始位置（左偏30像素）
        const int ICON_SIZE = 12;
        const int SPACING = 16;

        for (TowerDefense::Effect* effect : enemy->getActiveEffects()) {
            float remaining = effect->getRemainingTime();
            float duration = effect->getDuration();
            float progress = (duration > 0) ? (remaining / duration) : 0.0f;

            QPixmap icon;
            QColor color;
            switch (effect->getType()) {
            case TowerDefense::EffectType::POISON:
                icon = m_poisonIcon;
                color = Qt::magenta;
                break;
            case TowerDefense::EffectType::BURN:
                icon = m_burnIcon;
                color = Qt::red;
                break;
            case TowerDefense::EffectType::SLOW:
                icon = m_slowIcon;
                color = Qt::cyan;
                break;
            default:
                continue; // 未知效果类型，跳过
            }

            // 绘制图标（已预缩放，直接绘制）
            if (!icon.isNull()) {
                painter.drawPixmap(iconBase, icon);
            } else {
                painter.fillRect(QRectF(iconBase.x(), iconBase.y(), ICON_SIZE, ICON_SIZE), color);
            }

            // 绘制环形进度条（包围图标）
            QRectF ringRect(iconBase.x() - 2, iconBase.y() - 2, ICON_SIZE + 4, ICON_SIZE + 4);
            painter.setPen(QPen(Qt::white, 2));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(ringRect);

            // 从12点钟方向顺时针绘制弧（角度转换为16进制）
            int startAngle = 90 * 16;                                    // 12点钟位置
            int spanAngle = -static_cast<int>(progress * 360 * 16);      // 顺时针减少
            painter.setPen(QPen(color, 2));
            painter.drawArc(ringRect, startAngle, spanAngle);

            // 移动到下一个图标位置
            iconBase.rx() += SPACING;
        }
    }
}

void GameWidget::drawBullets(QPainter& painter) {
    const QList<TowerDefense::Bullet*>& bullets = m_gameManager->getBullets();
    for (TowerDefense::Bullet* bullet : bullets) {
        if (!bullet->isActive()) continue;
        const TowerDefense::WorldPos& pos = bullet->getWorldPos();
        QPixmap pix;
        // 判断是否为减速子弹
        if (dynamic_cast<TowerDefense::SlowBullet*>(bullet)) {
            pix = m_slowBulletPixmap;
        } else {
            pix = m_bulletPixmap;
        }
        painter.drawPixmap(QPointF(pos.x - 8, pos.y - 8), pix);
    }
}

void GameWidget::drawSelectedPreview(QPainter& painter) {
    if (!m_hasSelectedTower || !m_hoverValid) return;
    int cellSize = TowerDefense::Constants::CELL_SIZE;
    int row = m_hoverGrid.row;
    int col = m_hoverGrid.col;
    // 绘制半透明高亮边框
    QRect rect(col * cellSize, row * cellSize, cellSize, cellSize);
    painter.setPen(QPen(Qt::yellow, 3, Qt::DashLine));
    painter.setBrush(QColor(255, 255, 0, 50));
    painter.drawRect(rect);
    // 如果可建造，绿色边框，否则红色
    bool buildable = m_gameManager->canBuildTower(m_selectedTower, row, col); // 需GameManager提供此方法
    if (buildable) {
        painter.setPen(QPen(Qt::green, 2));
        painter.setBrush(Qt::NoBrush);
    } else {
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::NoBrush);
    }
    painter.drawRect(rect);
}

void GameWidget::drawPauseOverlay(QPainter& painter) {
    painter.fillRect(rect(), QColor(0, 0, 0, 150));
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(40);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, "暂停");
}

void GameWidget::drawHealthBar(QPainter& painter, const QPointF& pos, float current, float max, int width, int height) {
    if (max <= 0) return;
    float ratio = qBound(0.0f, current / max, 1.0f);
    QRectF bar(pos.x() - width/2, pos.y() - height - 2, width, height);
    painter.fillRect(bar, Qt::black);
    QRectF healthRect(bar.x() + 1, bar.y() + 1, (width - 2) * ratio, height - 2);
    QColor color = QColor::fromRgbF(1.0f - ratio, ratio, 0);
    painter.fillRect(healthRect, color);
}

void GameWidget::mouseMoveEvent(QMouseEvent* event) {
    int cellSize = TowerDefense::Constants::CELL_SIZE;
    int col = event->x() / cellSize;
    int row = event->y() / cellSize;
    if (row >= 0 && row < TowerDefense::Constants::ROWS &&
        col >= 0 && col < TowerDefense::Constants::COLS) {
        m_hoverGrid.row = row;
        m_hoverGrid.col = col;
        m_hoverValid = true;
    } else {
        m_hoverValid = false;
    }
    update();
}

void GameWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        int cellSize = TowerDefense::Constants::CELL_SIZE;
        int col = event->x() / cellSize;
        int row = event->y() / cellSize;
        if (row < 0 || row >= TowerDefense::Constants::ROWS ||
            col < 0 || col >= TowerDefense::Constants::COLS) {
            return;
        }
        if (!m_hasSelectedTower) {
            // 显示提示
            QMainWindow* mainWin = qobject_cast<QMainWindow*>(window());
            if (mainWin) {
                mainWin->statusBar()->showMessage("请先选择一种塔！", 2000);
            }
            return;
        }
        bool success = m_gameManager->buildTower(m_selectedTower, row, col);
        QMainWindow* mainWin = qobject_cast<QMainWindow*>(window());
        if (mainWin) {
            if (success) {
                mainWin->statusBar()->showMessage("塔建造成功！", 1500);
            } else {
                mainWin->statusBar()->showMessage("建造失败：资源不足或位置不可用！", 2000);
            }
        }
    } else if (event->button() == Qt::RightButton) {
        QPointF pos = event->position();
        // 检测点击位置是否在某个塔上
        const QList<TowerDefense::TowerBase*>& towers = m_gameManager->getTowers();
        for (TowerDefense::TowerBase* tower : towers) {
            if (!tower->isActive()) continue;
            const TowerDefense::WorldPos& tPos = tower->getWorldPos();
            float dist = std::hypot(pos.x() - tPos.x, pos.y() - tPos.y);
            if (dist < TowerDefense::Constants::CELL_SIZE * 0.5) {
                showTowerMenu(tower, event->globalPosition().toPoint());
                return;
            }
        }
        // 未点击到塔，清除选中
        clearSelectedTower();
        QWidget::mousePressEvent(event);
        return;
    }
}

void GameWidget::mouseReleaseEvent(QMouseEvent* event) {
    // 对于右键释放，处理取消选中（可选）
    if (event->button() == Qt::RightButton) {
        clearSelectedTower();
        // 通知父窗口取消按钮高亮（通过信号或直接调用）
        // 由于 GameWidget 不持有按钮，可通过 GamePage 调用
        // 这里只做自身状态重置
    }
    QWidget::mouseReleaseEvent(event);
}

void GameWidget::showTowerMenu(TowerDefense::TowerBase* tower, const QPoint& globalPos) {
    if (!tower) return;

    QMenu* menu = new QMenu(this);
    QAction* upgradeAction = nullptr;
    if (tower->canUpgrade()) {
        int cost = tower->getUpgradeCost();
        upgradeAction = menu->addAction(QString("升级（消耗%1资源）").arg(cost));
    }
    QAction* skillAction = nullptr;
    if (tower->hasActiveSkill()) {
        skillAction = menu->addAction("释放技能（就绪）");
        skillAction->setEnabled(tower->isSkillReady());
    }
    if (menu->actions().isEmpty()) {
        menu->addAction("无可执行操作")->setEnabled(false);
    }

    // 创建定时器，每 500ms 更新菜单项
    QTimer* timer = new QTimer(menu);  // 作为菜单子对象，菜单销毁时自动停止
    QObject::connect(timer, &QTimer::timeout, [=]() mutable {
        // 检查塔是否仍有效
        bool stillValid = false;
        for (TowerDefense::TowerBase* t : m_gameManager->getTowers()) {
            if (t == tower && t->isActive() && !t->isDead()) {
                stillValid = true;
                break;
            }
        }
        if (!stillValid) {
            menu->close();  // 塔已死亡，关闭菜单
            return;
        }

        // 更新技能按钮状态
        if (skillAction) {
            bool ready = tower->isSkillReady();
            skillAction->setEnabled(ready);
            if (ready) {
                skillAction->setText("释放技能（就绪）");
            } else {
                // 获取剩余冷却时间（需要从塔获取，假设有 getSkillCooldownRemaining 方法）
                // 若没有，可仅显示“冷却中”
                skillAction->setText("释放技能（冷却中）");
            }
        }
    });
    timer->start(500);

    QAction* chosen = menu->exec(globalPos);
    timer->stop();  // 可选，菜单关闭后停止定时器

    // ---- 辅助函数：获取主窗口状态栏 ----
    auto showStatus = [this](const QString& msg, int timeout = 1500) {
        QMainWindow* mainWin = qobject_cast<QMainWindow*>(window());
        if (mainWin) mainWin->statusBar()->showMessage(msg, timeout);
    };

    // 处理选择结果（同前）
    if (!chosen) {
        delete menu;
        return;
    }

    // ---- 升级分支 ----
    if (chosen == upgradeAction) {
        bool stillValid = false;
        for (TowerDefense::TowerBase* t : m_gameManager->getTowers()) {
            if (t == tower && t->isActive() && !t->isDead()) {
                stillValid = true;
                break;
            }
        }
        if (!stillValid) {
            showStatus("塔已损坏，无法升级！", 1500);
            delete menu;
            return;
        }

        if (m_gameManager->getResource() >= tower->getUpgradeCost()) {
            m_gameManager->addResource(-tower->getUpgradeCost());
            tower->doUpgrade();
            update();
            showStatus("塔升级成功！", 1500);
        } else {
            showStatus("资源不足！", 1500);
        }
    }
    // ---- 技能分支 ----
    else if (chosen == skillAction) {
        bool stillValid = false;
        for (TowerDefense::TowerBase* t : m_gameManager->getTowers()) {
            if (t == tower && t->isActive() && !t->isDead()) {
                stillValid = true;
                break;
            }
        }
        if (!stillValid) {
            showStatus("塔已损坏，无法释放技能！", 1500);
            delete menu;
            return;
        }
        if (!tower->isSkillReady()) {
            showStatus("技能仍在冷却中！", 1500);
            delete menu;
            return;
        }
        tower->activateSkill();
        update();
        showStatus("技能释放！", 1500);
    }

    delete menu; // 手动释放（若无父对象）
}



} // namespace TowerDefenseUI