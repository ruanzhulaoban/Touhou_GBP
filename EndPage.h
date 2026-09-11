#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>

namespace TowerDefense {
class GameManager;
}

namespace TowerDefenseUI {

class EndPage : public QWidget {
    Q_OBJECT

public:
    explicit EndPage(TowerDefense::GameManager* manager, QWidget* parent = nullptr);

    void setResult(bool win, float elapsedTime);

signals:
    void backToMenuRequested();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onBackClicked();

private:
    TowerDefense::GameManager* m_gameManager;
    QLabel* m_titleLabel;
    QLabel* m_infoLabel;
    QLabel* m_imageLabel;      // Q版小人
    QPushButton* m_backButton;
    QPixmap m_background;      // 背景图片
};

} // namespace TowerDefenseUI