cmake_minimum_required(VERSION 3.16)

project(TouhouGBP VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TouhouGBP
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ConfigLoader.h
        Constants.h
        DataStructs.h
        Enums.h
        ConfigLoader.cpp
        GameObject.h
        GameObject.cpp
        Enemy/EnemyBase.h
        Enemy/EnemyBase.cpp
        Tower/TowerBase.h
        Tower/TowerBase.cpp
        Bullet/Bullet.h
        Bullet/Bullet.cpp
        Effect/Effect.h
        Effect/Effect.cpp
        Map/GameMap.h
        Map/GameMap.cpp
        Wave_Level/WaveConfig.h
        Wave_Level/LevelManager.h
        Wave_Level/LevelManager.cpp
        Wave_Level/WaveConfig.cpp
        GameManager.cpp
        GameManager.h
        Enemy/EnemyBoss.h
        Enemy/EnemyFast.h
        Enemy/EnemyHeavy.h
        Enemy/EnemyNormal.h
        Enemy/EnemyNormal.h
        Enemy/EnemyResistant.h
        Enemy/EnemySummon.h
        Enemy/EnemyBoss.cpp
        Enemy/EnemyFast.cpp
        Enemy/EnemyHeavy.cpp
        Enemy/EnemyNormal.cpp
        Enemy/EnemyResistant.cpp
        Enemy/EnemySummon.cpp
        Tower/TowerAOE.h
        Tower/TowerArrow.h
        Tower/TowerBase.h
        Tower/TowerLaser.h
        Tower/TowerResource.h
        Tower/TowerSlow.h
        Tower/TowerWall.h
        Tower/TowerAOE.cpp
        Tower/TowerArrow.cpp
        Tower/TowerLaser.cpp
        Tower/TowerResource.cpp
        Tower/TowerSlow.cpp
        Tower/TowerWall.cpp
        Effect/BurnEffect.h
        Effect/PoisonEffect.h
        Effect/SlowEffect.h
        Effect/BurnEffect.cpp
        Effect/PoisonEffect.cpp
        Effect/SlowEffect.cpp
        GamePage.h
        GameWidget.h
        StartPage.h
        EndPage.h
        EndPage.cpp
        GamePage.cpp
        GameWidget.cpp
        StartPage.cpp


        build/config/game_config.json
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/bullet.png
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/dark_soil.png
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/enemy_boss.png
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/enemy_fast.png
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/enemy_heavy.png
        build/Desktop_Qt_6_5_3_MinGW_64_bit_Debug/assets/images/enemy_normal.png
        Bullet/SlowBullet.h
        Bullet/SlowBullet.cpp
        Audio/AudioManager.h
        Audio/AudioManager.cpp




    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TouhouGBP APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TouhouGBP SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TouhouGBP
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TouhouGBP PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TouhouGBP)
endif()
set_target_properties(TouhouGBP PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TouhouGBP
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TouhouGBP)
endif()
