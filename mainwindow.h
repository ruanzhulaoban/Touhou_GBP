#pragma once
#include <QMainWindow>
#include <QStackedWidget>

namespace TowerDefense {
class GameManager;
}

namespace TowerDefenseUI {

class StartPage;
class GamePage;
class EndPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(TowerDefense::GameManager* manager, QWidget* parent = nullptr);
    ~MainWindow();

    void switchToPage(int index); // 0: Start, 1: Game, 2: End

private slots:
    void onGameOver();   // 改为无参
    void onGameWin();
    void onStartGameRequested(int levelIndex);

private:
    void setupUI();

    TowerDefense::GameManager* m_gameManager;
    QStackedWidget* m_stackedWidget;
    StartPage* m_startPage;
    GamePage* m_gamePage;
    EndPage* m_endPage;
    int m_currentLevel;
};

} // namespace TowerDefenseUI