#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "GameManager.h"
#include "ConfigLoader.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 加载配置文件
    QString configPath = "D:\\qt\\TouhouGBP\\build\\config\\game_config.json";
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