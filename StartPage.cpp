#include "StartPage.h"
#include "GameManager.h"
#include <QVBoxLayout>
#include <QFont>
#include <QPainter>

namespace TowerDefenseUI {

StartPage::StartPage(TowerDefense::GameManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_gameManager(manager)
    , m_titleLabel(nullptr)
    , m_levelCombo(nullptr)
    , m_startButton(nullptr)
{
    // 加载背景图片（若失败则使用备用色）
    m_background.load("./assets/images/start_background.png");
    if (m_background.isNull()) {
        qWarning() << "StartPage: Failed to load background image, using fallback color.";
        m_background = QPixmap(1, 1);
        m_background.fill(QColor(40, 40, 60));
    }

    // 垂直居中布局，间距增大
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);   // 从20增加到30，更舒展

    // 标题（保持原样，但若需调整可改）
    m_titleLabel = new QLabel("东方少女幻乐团", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(64);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: black; background: transparent;");
    layout->addWidget(m_titleLabel);

    // ---- 关卡下拉框（扩大尺寸） ----
    m_levelCombo = new QComboBox(this);
    // 新的关卡名称
    m_levelCombo->addItem("教学关卡");
    m_levelCombo->addItem("雪原");
    m_levelCombo->addItem("丘陵");
    m_levelCombo->addItem("高原");
    m_levelCombo->setFixedWidth(300);                    // 宽度从200增加到300
    // 增大字体
    QFont comboFont = m_levelCombo->font();
    comboFont.setPointSize(16);
    m_levelCombo->setFont(comboFont);
    // 调整样式以适应更大尺寸
    m_levelCombo->setStyleSheet(
        "background: rgba(255,255,255,200); "
        "border-radius: 8px; "
        "padding: 10px;"
    );
    layout->addWidget(m_levelCombo, 0, Qt::AlignCenter);

    // ---- 开始按钮（扩大尺寸） ----
    m_startButton = new QPushButton("开始游戏", this);
    m_startButton->setFixedSize(240, 70);                // 从150x40扩大到240x70
    QFont btnFont = m_startButton->font();
    btnFont.setPointSize(18);
    m_startButton->setFont(btnFont);
    m_startButton->setStyleSheet(
        "background: rgba(230, 126, 34, 220); "
        "color: white; "
        "border-radius: 12px; "                          // 圆角从8px增加到12px
        "font-weight: bold;"
    );
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);

    connect(m_startButton, &QPushButton::clicked, this, &StartPage::onStartClicked);
}

void StartPage::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(rect(), m_background);
}

void StartPage::onStartClicked() {
    int levelIndex = m_levelCombo->currentIndex();
    emit startGameRequested(levelIndex);
}

} // namespace TowerDefenseUI