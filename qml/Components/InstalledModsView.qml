import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string filterQuery: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        // Top summary bar
        Rectangle {
            Layout.fillWidth: true
            height: 52
            radius: 10
            color: "#0F1714"
            border.width: 1
            border.color: "#1E2D26"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                Column {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 2

                    Text {
                        text: "Установленные моды"
                        color: "#FFFFFF"
                        font.family: "Inter"
                        font.pixelSize: 15
                        font.weight: Font.Bold
                    }

                    Text {
                        text: installedMods.totalInstalledCount + " модов • " +
                              installedMods.totalSizeText + " • " +
                              installedMods.enabledCount + " активно"
                        color: "#7E948A"
                        font.family: "Inter"
                        font.pixelSize: 11
                    }
                }

                Item { Layout.fillWidth: true }

                // Rescan button
                Rectangle {
                    width: rescanRow.implicitWidth + 20
                    height: 32
                    radius: 7
                    color: rescanMouse.containsMouse ? "#1C2A23" : "#141E1A"
                    border.width: 1
                    border.color: rescanMouse.containsMouse ? "#2E473A" : "#1E2B25"

                    Behavior on color { ColorAnimation { duration: 110 } }

                    Row {
                        id: rescanRow
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            text: "⟳"
                            color: "#1CE47E"
                            font.pixelSize: 14
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "Обновить"
                            color: "#D0DCD6"
                            font.family: "Inter"
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: rescanMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: app.rescanInstalledMods()
                    }
                }
            }
        }

        // Category Filter Chips & Search Bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Horizontal Scrollable Category Filter Chips
            Flickable {
                Layout.fillWidth: true
                height: 32
                contentWidth: chipsRow.implicitWidth
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Row {
                    id: chipsRow
                    spacing: 6

                    Repeater {
                        model: [
                            { slug: "all",     label: "Все" },
                            { slug: "cars",    label: "Легковые авто" },
                            { slug: "jeeps",   label: "Внедорожники" },
                            { slug: "trucks",  label: "Грузовики" },
                            { slug: "maps",    label: "Карты" },
                            { slug: "racing",  label: "Спорт" },
                            { slug: "unknown", label: "! неизвестный" }
                        ]

                        delegate: Rectangle {
                            id: chipBtn
                            readonly property bool isSelected: installedMods.categoryFilter === modelData.slug
                            readonly property int count: installedMods.countForCategory(modelData.slug)
                            readonly property bool isUnknown: modelData.slug === "unknown"

                            visible: modelData.slug === "all" || chipBtn.count > 0
                            height: 30
                            width: chipTextRow.implicitWidth + 16
                            radius: 7

                            color: isSelected
                                ? (isUnknown ? "#3D2411" : "#1B422F")
                                : (chipMouse.containsMouse ? "#18241E" : "#111815")
                            border.width: 1
                            border.color: isSelected
                                ? (isUnknown ? "#E58E26" : "#1CE47E")
                                : (chipMouse.containsMouse ? "#2A3D33" : "#1D2823")

                            Behavior on color { ColorAnimation { duration: 110 } }
                            Behavior on border.color { ColorAnimation { duration: 110 } }

                            Row {
                                id: chipTextRow
                                anchors.centerIn: parent
                                spacing: 6

                                Text {
                                    text: modelData.label
                                    color: chipBtn.isSelected
                                        ? (chipBtn.isUnknown ? "#F59E0B" : "#1CE47E")
                                        : (chipBtn.isUnknown ? "#D97706" : "#A6B8AE")
                                    font.family: "Inter"
                                    font.pixelSize: 11
                                    font.weight: chipBtn.isSelected ? Font.Bold : Font.Medium
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Rectangle {
                                    height: 16
                                    width: countTxt.implicitWidth + 8
                                    radius: 4
                                    color: chipBtn.isSelected
                                        ? (chipBtn.isUnknown ? "#593618" : "#153325")
                                        : "#16201B"
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        id: countTxt
                                        anchors.centerIn: parent
                                        text: chipBtn.count
                                        color: chipBtn.isSelected
                                            ? (chipBtn.isUnknown ? "#FBBF24" : "#1CE47E")
                                            : "#6B7F75"
                                        font.family: "JetBrains Mono"
                                        font.pixelSize: 10
                                        font.weight: Font.Bold
                                    }
                                }
                            }

                            MouseArea {
                                id: chipMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: installedMods.categoryFilter = modelData.slug
                            }
                        }
                    }
                }
            }

            // Compact Search Box for installed mods
            Rectangle {
                Layout.preferredWidth: 210
                height: 32
                radius: 7
                color: localSearchInput.activeFocus ? "#15201B" : "#101614"
                border.width: 1
                border.color: localSearchInput.activeFocus ? "#1CE47E" : "#1E2924"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 8
                    spacing: 6

                    Text {
                        text: "🔍"
                        font.pixelSize: 10
                        opacity: 0.6
                        Layout.alignment: Qt.AlignVCenter
                    }

                    TextInput {
                        id: localSearchInput
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        color: "#E8F0EC"
                        selectionColor: "#1CE47E"
                        selectedTextColor: "#061A10"
                        font.family: "Inter"
                        font.pixelSize: 11
                        clip: true

                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            text: "Поиск в модах..."
                            color: "#5C7066"
                            font: localSearchInput.font
                            visible: !localSearchInput.text && !localSearchInput.activeFocus
                        }

                        onTextChanged: installedMods.searchQuery = text
                    }

                    Text {
                        visible: localSearchInput.text.length > 0
                        text: "×"
                        color: "#8FA399"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                localSearchInput.text = "";
                                installedMods.searchQuery = "";
                            }
                        }
                    }
                }
            }
        }

        // Installed Mods List
        ListView {
            id: installedList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: installedMods

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                contentItem: Rectangle {
                    implicitWidth: 5
                    radius: 3
                    color: "#2C3D35"
                }
            }

            // Empty state placeholder
            Text {
                anchors.centerIn: parent
                visible: installedMods.totalInstalledCount === 0
                text: "В папке BeamNG.drive не найдено модов.\nУстановите моды из каталога или проверьте путь в настройках."
                color: "#6B7F76"
                font.family: "Inter"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                lineHeight: 1.4
            }

            delegate: Rectangle {
                id: itemCard
                width: installedList.width - 8
                height: 64
                radius: 9
                color: itemMouse.containsMouse ? "#131C18" : "#0E1512"
                border.width: 1
                border.color: !model.isEnabled ? "#362121" : (itemMouse.containsMouse ? "#22352B" : "#18241E")

                Behavior on color { ColorAnimation { duration: 110 } }
                Behavior on border.color { ColorAnimation { duration: 110 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 12

                    // Status Indicator Dot
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: model.isEnabled ? "#1CE47E" : "#E24A4A"
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Title & Filename
                    Column {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 3

                        Text {
                            text: model.title
                            color: model.isEnabled ? "#E6ECE9" : "#8A9992"
                            font.family: "Inter"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                            width: Math.min(parent.width, 420)
                        }

                        Text {
                            text: model.filename
                            color: "#5C7066"
                            font.family: "JetBrains Mono"
                            font.pixelSize: 11
                            elide: Text.ElideMiddle
                            width: Math.min(parent.width, 420)
                        }
                    }

                    // Category Pill with vibrant, distinct category themes
                    Rectangle {
                        readonly property string cat: model.categorySlug || "unknown"
                        readonly property bool isUnknown: cat === "unknown"
                        readonly property bool isJeep: cat === "jeeps"
                        readonly property bool isTruck: cat === "trucks"
                        readonly property bool isMap: cat === "maps"
                        readonly property bool isRacing: cat === "racing"

                        height: 22
                        width: catTxt.implicitWidth + 14
                        radius: 5
                        color: isUnknown ? "#281C10" :
                               isJeep ? "#0E2328" :
                               isTruck ? "#281E10" :
                               isMap ? "#101D2B" :
                               isRacing ? "#22122B" : "#12211A"
                        border.width: 1
                        border.color: isUnknown ? "#6E4B18" :
                                      isJeep ? "#194B54" :
                                      isTruck ? "#5C3A16" :
                                      isMap ? "#1B3E63" :
                                      isRacing ? "#4E2066" : "#1E3A2C"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            id: catTxt
                            anchors.centerIn: parent
                            text: model.categoryName || "Мод"
                            color: parent.isUnknown ? "#F59E0B" :
                                   parent.isJeep ? "#2DD4BF" :
                                   parent.isTruck ? "#FB923C" :
                                   parent.isMap ? "#60A5FA" :
                                   parent.isRacing ? "#C084FC" : "#34D399"
                            font.family: "Inter"
                            font.pixelSize: 11
                            font.weight: parent.isUnknown ? Font.DemiBold : Font.Normal
                        }
                    }

                    // Size Pill
                    Rectangle {
                        height: 22
                        width: sizeTxt.implicitWidth + 14
                        radius: 5
                        color: "#14201A"
                        border.width: 1
                        border.color: "#1E2F26"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            id: sizeTxt
                            anchors.centerIn: parent
                            text: model.fileSizeText
                            color: "#1CE47E"
                            font.family: "JetBrains Mono"
                            font.pixelSize: 11
                            font.weight: Font.Bold
                        }
                    }

                    // Toggle Switch Button (Вкл / Выкл)
                    Rectangle {
                        width: 74
                        height: 28
                        radius: 6
                        color: toggleMouse.containsMouse ? (model.isEnabled ? "#1A402D" : "#361D1D")
                                                         : (model.isEnabled ? "#143324" : "#281616")
                        border.width: 1
                        border.color: model.isEnabled ? "#1CE47E" : "#D44343"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: model.isEnabled ? "Включен" : "Отключен"
                            color: model.isEnabled ? "#1CE47E" : "#E26A6A"
                            font.family: "Inter"
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }

                        MouseArea {
                            id: toggleMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.toggleInstalledMod(model.filePath)
                        }
                    }

                    // Open in Folder button
                    Rectangle {
                        width: 30
                        height: 28
                        radius: 6
                        color: openMouse.containsMouse ? "#1F2D26" : "#141D19"
                        border.width: 1
                        border.color: openMouse.containsMouse ? "#2F4539" : "#1D2823"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: "📁"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: openMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.openInstalledModFolder(model.filePath)
                        }
                    }

                    // Delete button
                    Rectangle {
                        width: 30
                        height: 28
                        radius: 6
                        color: delMouse.containsMouse ? "#3A1A1A" : "#1D1414"
                        border.width: 1
                        border.color: delMouse.containsMouse ? "#632727" : "#2B1A1A"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: "🗑"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: delMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.deleteInstalledMod(model.filePath)
                        }
                    }
                }

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    z: -1
                }
            }
        }
    }
}
