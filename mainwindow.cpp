#include "MainWindow.h"
#include "StartPage.h"
#include "GamePage.h"
#include "GameWidget.h"
#include "EndPage.h"
#include "GameManager.h"
#include <QStatusBar>
#include "Audio/AudioManager.h"

namespace TowerDefenseUI {

MainWindow::MainWindow(TowerDefense::GameManager* manager, QWidget* parent)
    : QMainWindow(parent)
    , m_gameManager(manager)
    , m_stackedWidget(nullptr)
    , m_startPage(nullptr)
    , m_gamePage(nullptr)
    , m_endPage(nullptr)
    , m_currentLevel(0)
{
    setupUI();

    // 连接 GameManager 信号
    connect(m_gameManager, &TowerDefense::GameManager::gameOver, this, &MainWindow::onGameOver);
    connect(m_gameManager, &TowerDefense::GameManager::gameWin, this, &MainWindow::onGameWin);

    connect(m_endPage, &EndPage::backToMenuRequested, [this]() {
        m_gameManager->shutdownGame();   // 停止游戏，不重新开始
        switchToPage(0);
    });

    connect(m_startPage, &StartPage::startGameRequested, this, &MainWindow::onStartGameRequested);

}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("塔防大作战");
    resize(1024, 768);

    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 创建页面
    m_startPage = new StartPage(m_gameManager, this);
    m_gamePage = new GamePage(m_gameManager, this);
    m_endPage = new EndPage(m_gameManager, this);

    m_stackedWidget->addWidget(m_startPage);
    m_stackedWidget->addWidget(m_gamePage);
    m_stackedWidget->addWidget(m_endPage);

    // 默认显示开始页
    switchToPage(0);

    // 状态栏
    statusBar()->showMessage("欢迎来到塔防大作战！");
}

void MainWindow::switchToPage(int index) {
    if (index >= 0 && index < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(index);
        if (index == 0) {
            statusBar()->showMessage("选择关卡并开始游戏");
            AudioManager::instance()->stopBackgroundMusic();
        } else if (index == 1) {
            statusBar()->showMessage("游戏进行中...");
            // 根据关卡索引播放不同的背景音乐
            QString musicPath;
            switch (m_currentLevel) {
            case 0:
                musicPath = "./assets/audio/bg_music_level1.mp3";
                break;
            case 1:
                musicPath = "./assets/audio/bg_music_level2.mp3";
                break;
            case 2:
                musicPath = "./assets/audio/bg_music_level3.mp3";
                break;
            default:
                musicPath = "./assets/audio/bg_music.mp3";
                break;
            }
            AudioManager::instance()->playBackgroundMusic(musicPath, 0.4f);
        } else if (index == 2) {
            // 结束页，停止音乐（由 gameOver/gameWin 触发，已在槽中处理）
        }
    }
}


void MainWindow::onGameOver() {
    float time = m_gameManager->getElapsedTime();
    m_endPage->setResult(false, time);
    switchToPage(2);
}

void MainWindow::onGameWin() {
    float time = m_gameManager->getElapsedTime();
    m_endPage->setResult(true, time);
    switchToPage(2);
}

void MainWindow::onStartGameRequested(int levelIndex) {
    m_currentLevel = levelIndex;
    if (m_gameManager->initGame(levelIndex)) {
        switchToPage(1);
        statusBar()->showMessage("游戏开始！", 2000);
    } else {
        statusBar()->showMessage("关卡加载失败！", 2000);
    }
}

} // namespace TowerDefenseUI