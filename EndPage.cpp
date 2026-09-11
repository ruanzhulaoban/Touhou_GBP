#include "EndPage.h"
#include "GameManager.h"
#include <QVBoxLayout>
#include <QFont>
#include <QPainter>

namespace TowerDefenseUI {

EndPage::EndPage(TowerDefense::GameManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_gameManager(manager)
    , m_titleLabel(nullptr)
    , m_infoLabel(nullptr)
    , m_imageLabel(nullptr)
    , m_backButton(nullptr)
{
    // 加载背景图片
    m_background.load("./assets/images/end_background.png");
    if (m_background.isNull()) {
        qWarning() << "EndPage: Failed to load background image, using fallback color.";
        m_background = QPixmap(1, 1);
        m_background.fill(QColor(40, 40, 50));
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(15);

    // 标题
    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(36);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: white; background: transparent;");

    // Q版小人图片
    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setFixedSize(150, 150);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setStyleSheet("background: transparent;");

    // 信息标签
    m_infoLabel = new QLabel(this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    QFont infoFont = m_infoLabel->font();
    infoFont.setPointSize(16);
    m_infoLabel->setFont(infoFont);
    m_infoLabel->setStyleSheet("color: white; background: transparent;");

    // 返回按钮
    m_backButton = new QPushButton("返回主菜单", this);
    m_backButton->setFixedSize(160, 45);
    m_backButton->setStyleSheet(
        "QPushButton { background: rgba(230, 126, 34, 220); color: white; border-radius: 8px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background: rgba(230, 126, 34, 255); }"
        "QPushButton:pressed { background: rgba(200, 100, 20, 220); }"
    );

    layout->addWidget(m_titleLabel);
    layout->addWidget(m_imageLabel);
    layout->addWidget(m_infoLabel);
    layout->addWidget(m_backButton, 0, Qt::AlignCenter);

    connect(m_backButton, &QPushButton::clicked, this, &EndPage::onBackClicked);
}

void EndPage::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    // 拉伸背景图片填满整个窗口
    painter.drawPixmap(rect(), m_background);
}

void EndPage::setResult(bool win, float elapsedTime) {
    // 标题
    if (win) {
        m_titleLabel->setText("大胜利!");
        m_titleLabel->setStyleSheet("color: #00ff88; background: transparent;");
    } else {
        m_titleLabel->setText("失败...");
        m_titleLabel->setStyleSheet("color: #ff6666; background: transparent;");
    }
    m_infoLabel->setText(QString("用时: %1 秒").arg(elapsedTime, 0, 'f', 1));

    // 加载对应的Q版小人图片（与背景独立）
    QString imagePath;
    if (win) {
        imagePath = "./assets/images/win_character.png";
    } else {
        imagePath = "./assets/images/lose_character.png";
    }
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        m_imageLabel->clear();
        qWarning() << "EndPage: Failed to load image" << imagePath;
    } else {
        m_imageLabel->setPixmap(pixmap);
    }
}

void EndPage::onBackClicked() {
    emit backToMenuRequested();
}

} // namespace TowerDefenseUI