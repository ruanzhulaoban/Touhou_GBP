#include "GamePage.h"
#include "GameWidget.h"
#include "GameManager.h"
#include "Constants.h"
#include "ConfigLoader.h"
#include "Audio/AudioManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QIcon>
#include <QLabel>
#include <QToolTip>
#include <QStatusBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpression>

namespace TowerDefenseUI {

GamePage::GamePage(TowerDefense::GameManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_gameManager(manager)
    , m_gameWidget(nullptr)
    , m_resourceLabel(nullptr)
    , m_waveLabel(nullptr)
    , m_pauseButton(nullptr)
    , m_resetButton(nullptr)
    , m_cheatEdit(nullptr)
    , m_needRestartMusic(false)
{
    setupUI();
    connect(m_gameManager, &TowerDefense::GameManager::resourceChanged,
            this, &GamePage::onResourceChanged);
    connect(m_gameManager, &TowerDefense::GameManager::waveChanged,
            this, &GamePage::onWaveChanged);
    connect(m_gameWidget, &GameWidget::towerSelectionCleared,
            this, &GamePage::onTowerSelectionCleared);
    // 游戏逻辑更新后同步刷新渲染（替代 GameWidget 独立定时器）
    connect(m_gameManager, &TowerDefense::GameManager::gameUpdated,
            m_gameWidget, QOverload<>::of(&QWidget::update));
    onResourceChanged(m_gameManager->getResource());
    onWaveChanged(m_gameManager->getCurrentWave());
}

GamePage::~GamePage() {}

void GamePage::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶部工具栏 ----
    QWidget* toolbar = new QWidget(this);
    toolbar->setFixedHeight(60);
    toolbar->setStyleSheet("background-color: #2c3e50;");
    QHBoxLayout* toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(10, 0, 10, 0);
    toolLayout->setSpacing(15);

    // 资源标签
    m_resourceLabel = new QLabel("巧克力螺面包: 0", toolbar);
    m_resourceLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px;");
    toolLayout->addWidget(m_resourceLabel);

    // 波次标签
    m_waveLabel = new QLabel("波次: 0/5", toolbar);
    m_waveLabel->setStyleSheet("color: white; font-weight: bold; font-size: 14px;");
    toolLayout->addWidget(m_waveLabel);

    toolLayout->addStretch();

    // 暂停按钮
    m_pauseButton = new QPushButton("暂停", toolbar);
    m_pauseButton->setFixedSize(60, 40);
    m_pauseButton->setStyleSheet("background-color: #3498db; color: white; border-radius: 5px;");
    connect(m_pauseButton, &QPushButton::clicked, this, &GamePage::onPauseClicked);
    toolLayout->addWidget(m_pauseButton);

    // 重新开始按钮
    m_resetButton = new QPushButton("重新开始", toolbar);
    m_resetButton->setFixedSize(80, 40);
    m_resetButton->setStyleSheet("background-color: #e74c3c; color: white; border-radius: 5px;");
    connect(m_resetButton, &QPushButton::clicked, this, &GamePage::onResetClicked);
    toolLayout->addWidget(m_resetButton);

    // 作弊码输入框
    m_cheatEdit = new QLineEdit(toolbar);
    m_cheatEdit->setPlaceholderText("输入 +1000 增加资源");
    m_cheatEdit->setFixedWidth(150);
    m_cheatEdit->setStyleSheet("background-color: white; border-radius: 3px; padding: 2px;");
    connect(m_cheatEdit, &QLineEdit::returnPressed, this, &GamePage::onCheatReturnPressed);
    toolLayout->addWidget(m_cheatEdit);

    mainLayout->addWidget(toolbar);

    // ---- 主体：左侧游戏区 + 右侧塔面板 ----
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(20, 10, 20, 10);
    bodyLayout->setSpacing(0);

    // 左侧 GameWidget：固定尺寸并居中
    m_gameWidget = new GameWidget(m_gameManager, this);
    m_gameWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_gameWidget->setFixedSize(m_gameWidget->minimumSize()); // 使用地图实际尺寸
    bodyLayout->addWidget(m_gameWidget, 0, Qt::AlignCenter); // 水平垂直居中

    // 右侧塔面板（加宽以容纳文字 + 图片）
    QWidget* sidePanel = new QWidget(this);
    sidePanel->setFixedWidth(400);   // 原来200，现在加宽至330
    sidePanel->setStyleSheet("background-color: #34495e;");
    QVBoxLayout* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(8, 8, 8, 8);
    sideLayout->setSpacing(8);
    sideLayout->setAlignment(Qt::AlignTop);

    // 塔类型与对应的卡片名称（小写英文）
    m_towerTypes = {
        TowerDefense::TowerType::ARROW,
        TowerDefense::TowerType::SLOW,
        TowerDefense::TowerType::AOE,
        TowerDefense::TowerType::LASER,
        TowerDefense::TowerType::RESOURCE,
        TowerDefense::TowerType::WALL
    };
    QStringList cardNames = {"arrow", "slow", "aoe", "laser", "resource", "wall"};
    QStringList towerNames = {"花道和弹道高手小兰", "冰鸫三尺，太鸫了", "游吟诗人友希那的压场", "制霸第三次总选举的爱音光波", "巧克力螺会不会梦见里美", "筑紫的护盾"};
    // 描述文字（与名称一一对应）
    QStringList towerDescriptions = {
        "发射普通子弹，升级后可以齐射。招募酬金：10",
        "发射减速子弹，造成冰冻效果，升级后冷却时间缩短。招募酬金：10",
        "施加强压法阵，造成中毒效果，升级后可以清空附近敌人。招募酬金：20",
        "发射AnonBeam,造成灼烧效果，升级后冷却时间缩短。招募酬金：20",
        "从山吹面包房搬运巧克力螺包，升级后产量提高。招募酬金：10",
        "具有高生命值，阻挡敌人前进，升级后生命值提高且回满血。招募酬金：10"
    };
    for (int i = 0; i < m_towerTypes.size(); ++i) {
        // 每个塔项：水平布局（文字 | 图片）
        QWidget* itemWidget = new QWidget(sidePanel);
        itemWidget->setFixedHeight(150);   // 略小于按钮高度，确保边框完整显示
        QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->setAlignment(Qt::AlignVCenter);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(6);

        // 左侧文字标签
        QWidget* textWidget = new QWidget(itemWidget);
        textWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        QVBoxLayout* textLayout = new QVBoxLayout(textWidget);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(2);
        textLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        QLabel* nameLabel = new QLabel(towerNames[i], textWidget);
        nameLabel->setStyleSheet("color: white; font-weight: bold; font-size: 12px;");
        nameLabel->setWordWrap(true);
        textLayout->addWidget(nameLabel);

        QLabel* descLabel = new QLabel(towerDescriptions[i], textWidget);
        descLabel->setStyleSheet("color: #b0b0b0; font-size: 10px;");
        descLabel->setWordWrap(true);
        textLayout->addWidget(descLabel);

        itemLayout->addWidget(textWidget, 1); // 文字占据剩余空间

        // 右侧图片按钮
        QToolButton* btn = new QToolButton(itemWidget);
        btn->setCheckable(true);
        btn->setFixedSize(190, 144);

        // 加载卡面图片
        QString cardPath = QString("./assets/images/tower_%1_card.png").arg(cardNames[i]);
        QPixmap pixmap(cardPath);
        if (!pixmap.isNull()) {
            btn->setIcon(QIcon(pixmap));
            btn->setIconSize(QSize(180, 140));
        } else {
            btn->setText(towerNames[i]);
            qWarning() << "Failed to load card image:" << cardPath;
        }

        int cost = TowerDefense::ConfigLoader::getTowerCost(m_towerTypes[i]);
        btn->setToolTip(QString("%1\n费用: %2").arg(towerNames[i]).arg(cost));

        btn->setStyleSheet(
            "QToolButton { border: 2px solid transparent; background: transparent; }"
            "QToolButton:checked { "
            "border: 3px solid #e67e22; "
            "border-radius:2px;"
            "background: rgba(230, 126,34, 0.3); "
            "}"
            "QToolButton:hover { border: 2px solid #f1c40f; }"
            );

        connect(btn, &QToolButton::clicked, [this, i]() {
            onTowerButtonClicked(i);
        });

        // 关键：将按钮添加到列表，以便索引访问
        m_towerButtons.append(btn);

        // 将按钮添加到布局，并右对齐
        itemLayout->addWidget(btn, 0, Qt::AlignRight);

        // 将整个项添加到侧面板
        sideLayout->addWidget(itemWidget);
    }

    sideLayout->addStretch();   // 将按钮推至顶部
    bodyLayout->addWidget(sidePanel);
    mainLayout->addLayout(bodyLayout);

    // 初始取消所有塔按钮选中
    updateTowerButtons();
}

void GamePage::onResourceChanged(int resource) {
    m_resourceLabel->setText(QString("巧克力螺面包: %1").arg(resource));
    for (int i = 0; i < m_towerButtons.size(); ++i) {
        int cost = TowerDefense::ConfigLoader::getTowerCost(m_towerTypes[i]);
        m_towerButtons[i]->setEnabled(resource >= cost);
    }
}

void GamePage::onWaveChanged(int wave) {
    int total = m_gameManager->getTotalWaves();
    int displayWave = (wave >= total) ? total : (wave + 1);
    m_waveLabel->setText(QString("波次: %1/%2").arg(displayWave).arg(total));
}

void GamePage::onTowerButtonClicked(int id) {
    if (id < 0 || id >= m_towerButtons.size()) return;
    QAbstractButton* btn = m_towerButtons[id];
    // 总是设置为选中（不再 toggle）
    m_gameWidget->setSelectedTower(m_towerTypes[id]);
    // 取消其他按钮选中，当前按钮设为选中
    for (int i = 0; i < m_towerButtons.size(); ++i) {
        m_towerButtons[i]->setChecked(i == id);
    }
    // 更新样式（如果不需要可省略）
    updateTowerButtons();
}

void GamePage::updateTowerButtons() {
    // 样式由 checked 状态控制，无需额外操作
}

void GamePage::onPauseClicked() {
    m_gameManager->togglePause();
    bool paused = m_gameManager->isPaused();
    if (paused) {
        AudioManager::instance()->pauseBackgroundMusic();
        m_pauseButton->setText("继续");
        m_gameWidget->setPaused(true);
    } else {
        // 继续
        if (m_needRestartMusic) {
            // 从头播放当前关卡音乐
            int level = m_gameManager->getCurrentLevel();
            QString musicPath = QString("./assets/audio/bg_music_level%1.mp3").arg(level + 1);
            AudioManager::instance()->playBackgroundMusic(musicPath, 0.4f);
            m_needRestartMusic = false;
        } else {
            AudioManager::instance()->resumeBackgroundMusic();
        }
        m_pauseButton->setText("暂停");
        m_gameWidget->setPaused(false);
    }
}

void GamePage::onResetClicked() {
    bool wasPaused = m_gameManager->isPaused();

    // 停止当前音乐（无论是否暂停）
    AudioManager::instance()->stopBackgroundMusic();

    if (wasPaused) {
        // 暂停状态下重置，标记需要从头播放
        m_needRestartMusic = true;
    } else {
        // 未暂停状态下重置，立即从头播放（重置后游戏自动开始）
        m_needRestartMusic = false;
        int level = m_gameManager->getCurrentLevel();
        QString musicPath = QString("./assets/audio/bg_music_level%1.mp3").arg(level + 1);
        AudioManager::instance()->playBackgroundMusic(musicPath, 0.4f);
    }

    m_gameManager->resetGame();

    // 恢复暂停状态（若原本暂停）
    if (wasPaused) {
        m_gameManager->pauseGame();
        m_pauseButton->setText("继续");
        m_gameWidget->setPaused(true);
    } else {
        m_pauseButton->setText("暂停");
        m_gameWidget->setPaused(false);
    }

    // 清除选中塔
    m_gameWidget->clearSelectedTower();
    for (auto btn : m_towerButtons) btn->setChecked(false);
}

void GamePage::onCheatReturnPressed() {
    QString text = m_cheatEdit->text();
    QRegularExpression regex("[0-9]+");
    QRegularExpressionMatchIterator it = regex.globalMatch(text);
    QStringList parts;
    while (it.hasNext()) {
        parts << it.next().captured();
    }
    if (parts.isEmpty()) {
        QMessageBox::information(this, "作弊码", "请输入包含数字的字符串，例如 +1000");
        m_cheatEdit->clear();
        return;
    }
    int amount = parts.first().toInt();
    if (amount <= 0) amount = 1000;
    m_gameManager->cheatAddResource(amount);
    m_cheatEdit->clear();
    QMainWindow* mainWin = qobject_cast<QMainWindow*>(window());
    if (mainWin) {
        mainWin->statusBar()->showMessage(QString("作弊成功，增加 %1 资源").arg(amount), 2000);
    }
}

void GamePage::onTowerSelectionCleared() {
    for (auto btn : m_towerButtons) {
        btn->setChecked(false);
    }
}

} // namespace TowerDefenseUI