#pragma once
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

namespace TowerDefense {
class GameManager;
}

namespace TowerDefenseUI {

class StartPage : public QWidget {
    Q_OBJECT

public:
    explicit StartPage(TowerDefense::GameManager* manager, QWidget* parent = nullptr);

signals:
    void startGameRequested(int levelIndex);
protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onStartClicked();

private:
    TowerDefense::GameManager* m_gameManager;
    QLabel* m_titleLabel;
    QComboBox* m_levelCombo;
    QPushButton* m_startButton;
    QPixmap m_background;   // 背景图片
};

} // namespace TowerDefenseUI