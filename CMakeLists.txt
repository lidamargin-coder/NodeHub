cmake_minimum_required(VERSION 3.21)

project(NodeHub VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Network
    Widgets
)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(NodeHub
    src/main.cpp
    src/ModItem.h
    src/ModListModel.h
    src/ModListModel.cpp
    src/StmodsClient.h
    src/StmodsClient.cpp
    src/DownloadTask.h
    src/DownloadTask.cpp
    src/DownloadManager.h
    src/DownloadManager.cpp
    src/SettingsManager.h
    src/SettingsManager.cpp
    src/InstalledModsManager.h
    src/InstalledModsManager.cpp
    src/UpdateManager.h
    src/UpdateManager.cpp
    src/AppVersion.h
    src/AppController.h
    src/AppController.cpp
    app.rc
)

qt_add_resources(NodeHub "qml_resources"
    PREFIX "/NodeHub"
    FILES
        qml/Main.qml
        qml/Components/Sidebar.qml
        qml/Components/TopBar.qml
        qml/Components/ModCard.qml
        qml/Components/ModDetailsPanel.qml
        qml/Components/FolderSetupModal.qml
        qml/Components/DownloadsDrawer.qml
        qml/Components/InstalledModsView.qml
        fonts/Inter-Regular.ttf
        fonts/Inter-Medium.ttf
        fonts/Inter-Bold.ttf
        fonts/JetBrainsMono-Regular.ttf
        fonts/JetBrainsMono-Bold.ttf
        logos/logo3_hex_core.svg
        logos/app_icon_256.png
        logos/app.ico
)

set_target_properties(NodeHub PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
    QT_QML_MODULE_NO_IMPORT_SCAN TRUE
)

target_link_libraries(NodeHub PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Widgets
)

if(WIN32)
    target_link_libraries(NodeHub PRIVATE shell32)
endif()
