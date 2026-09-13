# TouhouGBP（东方 GBP）

> *A fan-made tower defense game where BanG Dream! girls fight enemies in a Touhou-featured way.*
> 一款以「东方Project」式弹幕塔防为核心玩法的同人塔防小游戏。

---

## ⚠️ 同人作品声明（版权说明）

本项目是一款**同人（二次创作）作品**，仅供学习交流与非商业用途，**与官方无关**。

- 游戏玩法、弹幕风格及部分角色/妖精立绘元素参考或取材自 **《东方Project》** 系列。
  - 《东方Project》及其相关角色、设定、音乐的著作权归 **上海爱丽丝幻乐团（上海アリス幻樂団 / Team Shanghai Alice，ZUN）** 所有。
- 游戏中的角色设定参考 **《BanG Dream!》** 系列。
  - 《BanG Dream!》及其角色、设定、音乐的著作权归 **Bushiroad / Craft Egg** 等相关权利方所有。

本项目**不包含、也不分发上述作品的任何官方素材**；项目中出现的图片、音频等资源均为作者个人用于学习演示而准备或占位使用的素材。

请遵守「东方Project」与《BanG Dream!》各自的**二次创作规范**；本作品**禁止用于任何商业用途**。如相关权利方提出要求，作者将立即删除相关内容。

---

## 简介

TouhouGBP 是一款基于 **Qt（Widgets）+ C++17 + CMake** 的 2D 塔防游戏：敌人沿地图自右向左行进，玩家在合法格子上建造防御塔，通过策略搭配塔与地形（黑土地、石地、冰面、传送门）抵御 5 波敌人、通关 3 个关卡。

游戏参数（塔、敌人、波次、关卡地图、初始资源等）均由 **JSON 配置文件** 驱动，可在不改代码的情况下调参。

### 主要特性

- **地图与地形**：12×16 网格；草地、黑土地（建造费 −30%）、石地（不可建造）、冰面（加速敌人但减速效果更强）、传送门（敌人被传送到配对格）。
- **资源系统**：单一资源「巧克力螺面包」，由资源塔产出；资源不足时建造会被拦截并提示。
- **6 类防御塔**：射手塔（单体穿透）、减速塔、范围毒塔（AOE）、激光塔（同直线灼烧）、资源塔、防御墙。
- **6 类敌人**：普通、快速、重甲、抗性（抗减速）、召唤（死亡分裂）、Boss（两阶段、二阶段强化）。
- **状态效果**：减速 / 中毒 / 灼烧，均有持续时间并自动失效。
- **波次与关卡**：5 波 / 关，3 个关卡（地图、初始资源、波次各异）；通关后可进入下一关或返回选关。
- **成长系统**：防御塔可升级（1→3 级），部分塔拥有主动技能（齐射、秒杀等）。
- **体验**：暂停/继续/重开、作弊码、实时显示资源/波次/关卡、血条与受击反馈，普通场景稳定 30 FPS 以上。

---

## 环境要求

| 依赖 | 说明 |
| --- | --- |
| Qt | **5.12+ 或 6.x**（需要 `Widgets` 与 `Multimedia` 两个模块） |
| CMake | ≥ 3.16 |
| 编译器 | 支持 **C++17**（MSVC / MinGW / GCC / Clang 均可） |

> 项目通过 `find_package(QT NAMES Qt6 Qt5 ...)` 自动适配 Qt5 / Qt6。多媒体模块用于背景音乐与音效播放。

---

## 目录结构

```
TouhouGBP/
├── main.cpp                 # 入口：加载配置 → 创建 GameManager → 显示主窗口
├── mainwindow.{cpp,h,ui}    # QStackedWidget 主窗口（页面切换 + 信号槽连接）
├── ConfigLoader.{cpp,h}     # JSON 配置加载器（静态单例风格，带缓存与默认值兜底）
├── GameManager.{cpp,h}      # 游戏总控（QTimer 游戏循环、胜负判定、对象管理）
├── Constants.h / DataStructs.h / Enums.h   # 常量、坐标结构、枚举
├── GameObject.{cpp,h}       # 游戏对象基类
├── StartPage / GamePage / GameWidget / EndPage   # 表现层页面与渲染
├── Audio/                   # 背景音乐 + 音效管理
├── Bullet/                  # 子弹 / 减速子弹
├── Enemy/                   # 敌人基类 + 6 种子类
├── Effect/                  # 状态效果（减速/中毒/灼烧）
├── Map/                     # 地图（地形 + 占用 + 传送门配对）
├── Tower/                   # 塔基类 + 6 种子类
├── Wave_Level/              # 波次与关卡数据解析
├── config/                  # JSON 配置（game_config.json）
├── assets/                  # 图片与音频资源（images/ + audio/）
├── CMakeLists.txt           # 构建脚本
└── .gitignore               # 忽略 build/ 等构建产物
```

---

## 构建与运行

### 方法一：Qt Creator

1. 打开 Qt Creator，选择「打开项目」，选中根目录的 `CMakeLists.txt`。
2. 选择一套已安装的 Qt 套件（Kit），确认勾选了 `Widgets` 与 `Multimedia` 模块。
3. 点击构建并运行。

### 方法二：命令行（CMake）

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64
cmake --build build
```

---

## ⚠️ 运行前必读：相对路径与工作目录

配置与资源已与 `build/` 分离，全部改为**相对路径**加载，位于项目根目录：

- 配置：`config/game_config.json`（`main.cpp` 中以 `"config/game_config.json"` 加载）
- 资源：`assets/images/*.png`、`assets/audio/*.mp3`（代码中以 `./assets/...` 加载）

由于使用相对路径，**程序的工作目录必须是项目根目录**：

- **Qt Creator**：在「运行 → Working directory」中设为项目根目录（可填 `%{sourceDir}`）。
- **命令行**：在项目根目录下运行编译产物；或把 `config/`、`assets/` 复制到可执行文件所在目录再运行。

> 若某张图片加载失败，游戏会退化为纯色方块占位（不会崩溃）；若配置缺失，程序会弹窗报错并退出。

---

## 配置说明

游戏全部数值由 `config/game_config.json` 驱动，顶层结构：

- `towers`：6 类塔的伤害、攻速、射程、造价、升级费用、技能冷却等。
- `enemies`：6 类敌人的血量、速度、伤害、攻击间隔、减速抗性等。
- `waves`：各波次敌人的种类 / 数量 / 生成间隔。
- `levels`：3 个关卡的地图（12×16 地形矩阵，0=草地 1=黑土地 2=石地 3=冰面 4=传送门）、初始资源、波次等。

---

## 许可证

- 代码采用 [MIT License](LICENSE)。
- 请注意：**许可证仅覆盖代码本身**，不涵盖项目中参考的第三方 IP（东方Project、BanG Dream! 等）以及相关素材，请务必遵守其权利方的二次创作规范（见上方「同人作品声明」）。
