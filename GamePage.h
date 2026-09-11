#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include "Enums.h"

namespace TowerDefense {
class GameManager;
}

namespace TowerDefenseUI {

class GameWidget;

class GamePage : public QWidget {
    Q_OBJECT

public:
    explicit GamePage(TowerDefense::GameManager* manager, QWidget* parent = nullptr);
    ~GamePage();

    GameWidget* getGameWidget() const { return m_gameWidget; }

private slots:
    void onResourceChanged(int resource);
    void onWaveChanged(int wave);
    void onTowerButtonClicked(int id);
    void onPauseClicked();
    void onResetClicked();
    void onCheatReturnPressed();
    void onTowerSelectionCleared();   // 新增

private:
    void setupUI();
    void updateTowerButtons();

    TowerDefense::GameManager* m_gameManager;
    GameWidget* m_gameWidget;

    QLabel* m_resourceLabel;
    QLabel* m_waveLabel;
    QPushButton* m_pauseButton;
    QPushButton* m_resetButton;
    QLineEdit* m_cheatEdit;

    QList<QAbstractButton*> m_towerButtons;   // 改为抽象基类，兼容QToolButton
    QList<TowerDefense::TowerType> m_towerTypes;

    bool m_needRestartMusic;
};

} // namespace TowerDefenseUI