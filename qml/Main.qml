import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Effects
import "Components"

Window {
    id: rootWindow
    width: 1240
    height: 760
    minimumWidth: 1240
    maximumWidth: 1240
    minimumHeight: 760
    maximumHeight: 760
    visible: true
    title: "NodeHub"
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    Connections {
        target: app
        function onRequestFolderSetup() {
            folderModal.openModal(false);
        }
    }

    // Main Rounded Window Shell (Locked Size 1240x760)
    Rectangle {
        id: shell
        anchors.fill: parent
        radius: 12
        color: "#070C0A"
        border.width: 1
        border.color: "#21312A"

        // Ambient lights layer (blurred with MultiEffect)
        Item {
            id: ambientBgSource
            anchors.fill: parent
            visible: false
            layer.enabled: true

            // Top-center emerald aura
            Rectangle {
                width: 820
                height: 440
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: -180
                radius: 410
                color: "#301CE47E"
            }

            // Bottom-left mint/teal ambient light
            Rectangle {
                width: 580
                height: 400
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.bottomMargin: -140
                anchors.leftMargin: -120
                radius: 290
                color: "#220EA868"
            }

            // Right-center soft ambient light
            Rectangle {
                width: 500
                height: 500
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: -170
                radius: 250
                color: "#1C159458"
            }
        }

        // Slightly blurred background effect
        MultiEffect {
            anchors.fill: parent
            source: ambientBgSource
            blurEnabled: true
            blur: 0.90
            blurMax: 64
            opacity: 0.90
        }

        // Vignette gradient overlay for depth & readability
        Rectangle {
            anchors.fill: parent
            radius: 12
            color: "transparent"
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#40060A08" }
                GradientStop { position: 0.45; color: "#10060A08" }
                GradientStop { position: 1.0; color: "#60040806" }
            }
        }

        // Smooth Corner Mask for child components (ensures smooth anti-aliased corners)
        Rectangle {
            id: contentMask
            anchors.fill: parent
            anchors.margins: 1
            radius: 11
            color: "white"
            visible: false
            layer.enabled: true
        }

        // Masked Container for all window content
        Item {
            id: contentContainer
            anchors.fill: parent
            anchors.margins: 1
            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: contentMask
            }

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // Left Category Sidebar
                Sidebar {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 224
                    Layout.minimumWidth: 224
                    Layout.maximumWidth: 224
                    targetWindow: rootWindow
                    currentCategory: app.currentCategory
                    onCategorySelected: function(slug) {
                        if (slug === "__open_folder_modal__") {
                            folderModal.openModal(true);
                            return;
                        }
                        topBar.searchText = "";
                        app.selectCategory(slug);
                    }
                }

                // Main Area: Full-width TopBar + Content Rows below
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 0

                    // Full-width TopBar extending to the far right edge of the window
                    TopBar {
                        id: topBar
                        Layout.fillWidth: true
                        targetWindow: rootWindow
                        downloadsOpen: downloadsDrawer.isOpen
                        onSearchSubmitted: function(query) {
                            app.search(query);
                        }
                        onSettingsClicked: folderModal.openModal(true)
                        onToggleDownloads: downloadsDrawer.isOpen = !downloadsDrawer.isOpen
                        onMinimizeClicked: rootWindow.showMinimized()
                        onCloseClicked: rootWindow.close()
                    }

                    // Automatic Background Update Banner
                    Rectangle {
                        id: updateBanner
                        Layout.fillWidth: true
                        visible: updateManager.isUpdating
                        implicitHeight: visible ? 42 : 0
                        color: updateManager.state === "ready" ? "#0F3D26" : "#0A1F16"
                        border.width: 1
                        border.color: updateManager.state === "ready" ? "#28F08B" : "#1CE47E"
                        clip: true

                        Behavior on implicitHeight { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
                        Behavior on color { ColorAnimation { duration: 160 } }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 16
                            spacing: 12

                            // High-tech glowing pulse dot
                            Rectangle {
                                width: 10
                                height: 10
                                radius: 5
                                color: updateManager.state === "ready" ? "#28F08B" : "#1CE47E"
                                Layout.alignment: Qt.AlignVCenter

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    running: updateBanner.visible
                                    NumberAnimation { from: 1.0; to: 0.25; duration: 600; easing.type: Easing.InOutQuad }
                                    NumberAnimation { from: 0.25; to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                                }
                            }

                            // Update Status Message
                            Text {
                                Layout.fillWidth: true
                                text: updateManager.statusText
                                color: "#FFFFFF"
                                font.family: "Inter"
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                elide: Text.ElideRight
                                Layout.alignment: Qt.AlignVCenter
                            }

                            // Progress Bar when downloading
                            Rectangle {
                                visible: updateManager.state === "downloading"
                                Layout.preferredWidth: 160
                                height: 6
                                radius: 3
                                color: "#14281F"
                                Layout.alignment: Qt.AlignVCenter

                                Rectangle {
                                    height: parent.height
                                    radius: 3
                                    width: parent.width * Math.min(1.0, Math.max(0.04, updateManager.progressPercent / 100.0))
                                    color: "#1CE47E"

                                    Behavior on width { NumberAnimation { duration: 120 } }
                                }
                            }

                            // Percentage text
                            Text {
                                visible: updateManager.state === "downloading"
                                text: updateManager.progressPercent + "%"
                                color: "#1CE47E"
                                font.family: "JetBrains Mono"
                                font.pixelSize: 11
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }

                            // Cancel update button (optional, only during downloading)
                            Rectangle {
                                visible: updateManager.state === "downloading"
                                width: 22
                                height: 22
                                radius: 11
                                color: cancelUpdMouse.containsMouse ? "#2E1A1A" : "transparent"
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"
                                    color: cancelUpdMouse.containsMouse ? "#FF7373" : "#8FA399"
                                    font.pixelSize: 11
                                }

                                MouseArea {
                                    id: cancelUpdMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: updateManager.cancelUpdate()
                                }
                            }
                        }
                    }

                    // Content row: Grid on left, Details Panel on right
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 0

                        // Installed Mods Management View
                        InstalledModsView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            visible: app.currentCategory === "installed"
                        }

                        // Center Grid Container
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            visible: app.currentCategory !== "installed"

                            GridView {
                                id: modGrid
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 2
                                anchors.topMargin: 4
                                anchors.bottomMargin: 8
                                clip: true

                                readonly property int availableGridWidth: width - 10
                                readonly property int cols: Math.max(2, Math.floor(availableGridWidth / 190))
                                cellWidth: Math.floor(availableGridWidth / cols)
                                cellHeight: Math.round(cellWidth * 0.84)

                                model: modModel

                                ScrollBar.vertical: ScrollBar {
                                    id: gridScrollBar
                                    policy: ScrollBar.AsNeeded
                                    width: 6
                                    anchors.right: parent.right

                                    contentItem: Rectangle {
                                        implicitWidth: 6
                                        radius: 3
                                        color: gridScrollBar.pressed ? "#1CE47E" : (gridScrollBar.hovered ? "#355444" : "#1F2E27")
                                        Behavior on color { ColorAnimation { duration: 120 } }
                                    }
                                }

                                delegate: ModCard {
                                    width: modGrid.cellWidth
                                    height: modGrid.cellHeight
                                    modId: model.modId
                                    title: model.title
                                    author: model.author
                                    version: model.version
                                    gameVersion: model.gameVersion
                                    thumbnailUrl: model.thumbnailUrl
                                    selected: app.selectedModId === model.modId
                                    onClicked: app.selectMod(model.modId, model.modUrl, model.categorySlug)
                                }

                                footer: Item {
                                    width: modGrid.width
                                    height: (app.hasMore || app.loadingList) ? 56 : 8

                                    Rectangle {
                                        visible: app.hasMore && !app.loadingList
                                        anchors.centerIn: parent
                                        width: 170
                                        height: 34
                                        radius: 8
                                        color: moreMouse.containsMouse ? "#1A2B23" : "#121C17"
                                        border.width: 1
                                        border.color: moreMouse.containsMouse ? "#1CE47E" : "#24362E"

                                        Text {
                                            anchors.centerIn: parent
                                            text: "Загрузить ещё"
                                            color: moreMouse.containsMouse ? "#FFFFFF" : "#A6BAB0"
                                            font.family: "Inter"
                                            font.pixelSize: 12
                                            font.weight: Font.Medium
                                        }

                                        MouseArea {
                                            id: moreMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: app.loadMore()
                                        }
                                    }

                                    BusyIndicator {
                                        anchors.centerIn: parent
                                        running: app.loadingList && modModel.count > 0
                                        visible: running
                                        width: 28
                                        height: 28
                                    }
                                }

                                onContentYChanged: {
                                    if (contentHeight > height && contentY + height >= contentHeight - 160) {
                                        if (app.hasMore && !app.loadingList) {
                                            app.loadMore();
                                        }
                                    }
                                }
                            }

                            // Initial empty/loading state
                            BusyIndicator {
                                anchors.centerIn: parent
                                running: app.loadingList && modModel.count === 0
                                visible: running
                            }

                            Text {
                                anchors.centerIn: parent
                                visible: !app.loadingList && modModel.count === 0
                                text: "Моды не найдены"
                                color: "#6C8076"
                                font.family: "Inter"
                                font.pixelSize: 14
                            }
                        }

                        // Right Detail Inspector
                        ModDetailsPanel {
                            Layout.fillHeight: true
                            width: 372
                            visible: app.currentCategory !== "installed"
                            detail: app.selectedModDetail
                            loading: app.loadingDetail
                            onInstallClicked: app.installSelectedMod()
                            onCancelDownloadClicked: downloadManager.cancelDownload(app.selectedModId)
                        }
                    }
                }
            }
        }

        // Overlay to close Downloads Drawer on outside click
        MouseArea {
            anchors.fill: parent
            visible: downloadsDrawer.isOpen
            z: 75
            onClicked: downloadsDrawer.isOpen = false
        }

        // Active Downloads Floating Flyout Drawer
        DownloadsDrawer {
            id: downloadsDrawer
            anchors.right: parent.right
            anchors.rightMargin: 14
            y: isOpen ? 60 : 48
            Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.OutQuad } }
            z: 80
        }

        // Animated Notification Toast Banner
        Rectangle {
            id: toast
            readonly property bool hasMsg: app.notificationText.length > 0
            property string lastMsg: ""
            property bool lastError: false

            visible: opacity > 0.001
            opacity: hasMsg ? 1.0 : 0.0
            scale: hasMsg ? 1.0 : 0.92

            // Floating positioning with smooth slide up
            anchors.horizontalCenter: parent.horizontalCenter
            y: hasMsg ? parent.height - height - 24 : parent.height + 30

            Behavior on y { NumberAnimation { duration: 280; easing.type: Easing.OutCubic } }
            Behavior on opacity { NumberAnimation { duration: 220 } }
            Behavior on scale { NumberAnimation { duration: 260; easing.type: Easing.OutBack } }

            width: Math.min(parent.width - 80, toastRow.implicitWidth + 32)
            height: Math.max(42, toastRow.implicitHeight + 18)
            radius: 11
            color: (toast.hasMsg ? app.notificationError : toast.lastError) ? "#2B1212" : "#0E241B"
            border.width: 1
            border.color: (toast.hasMsg ? app.notificationError : toast.lastError) ? "#E55353" : "#1CE47E"
            z: 90

            Connections {
                target: app
                function onNotificationChanged() {
                    if (app.notificationText.length > 0) {
                        toast.lastMsg = app.notificationText;
                        toast.lastError = app.notificationError;
                    }
                }
            }

            RowLayout {
                id: toastRow
                anchors.centerIn: parent
                spacing: 11

                // Status Icon badge
                Rectangle {
                    width: 22
                    height: 22
                    radius: 11
                    color: (toast.hasMsg ? app.notificationError : toast.lastError) ? "#4A1818" : "#163E2B"
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        anchors.centerIn: parent
                        text: (toast.hasMsg ? app.notificationError : toast.lastError) ? "!" :
                              ((toast.hasMsg ? app.notificationText : toast.lastMsg).indexOf("Запуск") !== -1 ? "▶" : "✓")
                        color: (toast.hasMsg ? app.notificationError : toast.lastError) ? "#FF7373" : "#1CE47E"
                        font.pixelSize: 11
                        font.bold: true
                    }
                }

                Text {
                    Layout.maximumWidth: 440
                    text: toast.hasMsg ? app.notificationText : toast.lastMsg
                    color: "#FFFFFF"
                    font.family: "Inter"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    visible: !(toast.hasMsg ? app.notificationError : toast.lastError) &&
                             (toast.hasMsg ? app.notificationText : toast.lastMsg).indexOf("установлен") !== -1
                    width: openFolderTxt.implicitWidth + 16
                    height: 26
                    radius: 6
                    color: "#1CE47E"

                    Text {
                        id: openFolderTxt
                        anchors.centerIn: parent
                        text: "Открыть папку"
                        color: "#041B0E"
                        font.family: "Inter"
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: app.openTargetFolderForSelected()
                    }
                }

                Text {
                    text: "✕"
                    color: "#8FA399"
                    font.pixelSize: 13
                    Layout.alignment: Qt.AlignVCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: app.clearNotification()
                    }
                }
            }

            Timer {
                interval: 5500
                running: toast.hasMsg
                onTriggered: app.clearNotification()
            }
        }

        // First-run / Settings Folder Setup Modal
        FolderSetupModal {
            id: folderModal
            anchors.fill: parent
            z: 100
        }
    }
}
