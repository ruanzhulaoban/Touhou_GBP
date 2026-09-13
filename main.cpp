#include <QApplication>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include "MainWindow.h"
#include "GameManager.h"
#include "ConfigLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 定位配置文件：优先可执行文件目录，其次 Qt Creator 的 build 目录上溯到源目录，
    // 最后退回当前工作目录，避免因工作目录不同而找不到 config/game_config.json。
    const QString relativePath = QStringLiteral("config/game_config.json");
    const QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/" + relativePath,   // 与 exe 同目录（部署后）
        QCoreApplication::applicationDirPath() + "/../" + relativePath, // build 目录 → 源目录（开发时）
        QDir::currentPath() + "/" + relativePath,                      // 当前工作目录
    };
    QString configPath;
    for (const QString& c : candidates) {
        if (QFileInfo::exists(c)) {
            configPath = c;
            break;
        }
    }
    if (configPath.isEmpty()) {
        configPath = relativePath; // 兜底，让 loadConfig 输出清晰的错误信息
    }

    if (!TowerDefense::ConfigLoader::loadConfig(configPath)) {
        QMessageBox::critical(nullptr, "配置加载失败",
                              QString("无法加载配置文件:\n%1\n\n请检查路径。").arg(configPath));
        return -1;
    }

    // 创建游戏管理器（不初始化关卡，由用户点击触发）
    TowerDefense::GameManager* manager = new TowerDefense::GameManager();

    // 显示主窗口
    TowerDefenseUI::MainWindow mainWindow(manager);
    mainWindow.show();

    return app.exec();
}