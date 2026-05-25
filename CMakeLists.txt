cmake_minimum_required(VERSION 3.16)

project(KursachSPZ VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Charts)
qt_standard_project_setup()

# Windows API Libraries — додано wtsapi32 для WTSRegisterSessionNotification
set(WIN_LIBS user32 gdi32 shell32 wtsapi32)

add_executable(KursachSPZ
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/core/databasemanager.cpp
    src/core/databasemanager.h
    src/ui/activitychart.cpp
    src/ui/activitychart.h
    src/monitoring/appmonitor.cpp
    src/monitoring/appmonitor.h
    src/monitoring/inputmonitor.cpp
    src/monitoring/inputmonitor.h
    src/monitoring/usbmonitor.cpp
    src/monitoring/usbmonitor.h
    src/monitoring/clipboardmonitor.cpp
    src/monitoring/clipboardmonitor.h
    src/monitoring/powermonitor.cpp
    src/monitoring/powermonitor.h
    src/ui/mainwindow.ui
)

target_link_libraries(KursachSPZ PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    ${WIN_LIBS}
)

target_include_directories(KursachSPZ PRIVATE
    src/ui
    src/core
    src/monitoring
    ${Qt6_Charts_INCLUDE_DIRS}
    ${Qt6_Core_INCLUDE_DIRS}
    ${Qt6_Gui_INCLUDE_DIRS}
    ${Qt6_Widgets_INCLUDE_DIRS}
    ${Qt6_Sql_INCLUDE_DIRS}
)
