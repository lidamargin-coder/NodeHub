import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent
    visible: opacity > 0.001
    opacity: 0.0
    z: 100

    property bool allowClose: true

    function openModal(canClose) {
        allowClose = canClose;
        pathField.text = settingsMgr.gamePath;
        errText.visible = false;
        opacity = 1.0;
        card.scale = 1.0;
    }

    function closeModal() {
        if (allowClose || settingsMgr.hasGamePath) {
            opacity = 0.0;
            card.scale = 0.95;
        }
    }

    function doSave() {
        var raw = pathField.text.trim();
        var clean = raw.replace(/^["']|["']$/g, '').trim();

        if (clean.length === 0) {
            errText.text = "Укажите путь к папке игры.";
            errText.visible = true;
            return;
        }

        if (settingsMgr.setGamePath(clean)) {
            errText.visible = false;
            root.closeModal();
        } else {
            errText.text = "Не удалось создать или найти указанную папку. Проверьте путь.";
            errText.visible = true;
        }
    }

    Behavior on opacity { NumberAnimation { duration: 150 } }

    // Dimmed backdrop
    Rectangle {
        anchors.fill: parent
        color: "#B3050807"

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (root.allowClose)
                    root.closeModal();
            }
        }
    }

    // Modal card with smooth scale
    Rectangle {
        id: card
        anchors.centerIn: parent
        width: 500
        height: contentCol.implicitHeight + 48
        radius: 14
        color: "#111916"
        border.width: 1
        border.color: "#24362E"
        scale: 0.95

        Behavior on scale { NumberAnimation { duration: 160; easing.type: Easing.OutQuad } }

        // Prevent clicks on card from bubbling to backdrop
        MouseArea {
            anchors.fill: parent
            onClicked: {}
        }

        ColumnLayout {
            id: contentCol
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 24
            spacing: 16

            // Header title + Close X
            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: "Папка игры BeamNG.drive"
                    color: "#FFFFFF"
                    font.family: "Inter"
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }

                Rectangle {
                    visible: root.allowClose
                    width: 28
                    height: 28
                    radius: 7
                    color: xMouse.containsMouse ? "#2B1616" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        color: xMouse.containsMouse ? "#FF6B6B" : "#8FA399"
                        font.pixelSize: 13
                    }

                    MouseArea {
                        id: xMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.closeModal()
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "Укажите корневую папку BeamNG.drive. Архивы с автомобилями будут автоматически распаковываться в content/vehicles, а карты — в content/levels."
                color: "#9DB0A6"
                font.family: "Inter"
                font.pixelSize: 12
                lineHeight: 1.4
                wrapMode: Text.WordWrap
            }

            // Path Input Field
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 8
                color: pathField.activeFocus ? "#15221C" : "#0E1512"
                border.width: 1
                border.color: errText.visible ? "#E55353" : (pathField.activeFocus ? "#1CE47E" : "#1E2C25")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    spacing: 8

                    TextInput {
                        id: pathField
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        color: "#FFFFFF"
                        selectionColor: "#1CE47E"
                        selectedTextColor: "#061A10"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 12
                        clip: true
                        selectByMouse: true

                        Text {
                            anchors.fill: parent
                            verticalAlignment: Text.AlignVCenter
                            text: "Например: D:\\Games\\BeamNG.drive"
                            color: "#4A5D54"
                            font: pathField.font
                            visible: !pathField.text && !pathField.activeFocus
                        }

                        onAccepted: root.doSave()
                    }

                    Rectangle {
                        width: browseTxt.implicitWidth + 14
                        height: 26
                        radius: 6
                        color: browseMouse.containsMouse ? "#1E3027" : "#14201A"
                        border.width: 1
                        border.color: "#24362E"

                        Text {
                            id: browseTxt
                            anchors.centerIn: parent
                            text: "Обзор..."
                            color: "#D0DCD6"
                            font.family: "Inter"
                            font.pixelSize: 11
                        }

                        MouseArea {
                            id: browseMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var chosen = settingsMgr.browseGameFolder();
                                if (chosen && chosen.length > 0) {
                                    pathField.text = chosen;
                                    errText.visible = false;
                                }
                            }
                        }
                    }
                }
            }

            // Error notice
            Text {
                id: errText
                Layout.fillWidth: true
                visible: false
                color: "#FF6B6B"
                font.family: "Inter"
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }

            // Target mapping preview
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: mapCol.implicitHeight + 16
                radius: 8
                color: "#0B110E"
                border.width: 1
                border.color: "#18241E"

                ColumnLayout {
                    id: mapCol
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4

                    Text {
                        text: "Куда будут попадать моды:"
                        color: "#7E9489"
                        font.family: "Inter"
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "• Автомобили ➔ \\content\\vehicles"
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                    }

                    Text {
                        text: "• Карты ➔ \\content\\levels"
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                    }

                    Text {
                        text: "• Прочее ➔ \\content"
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 11
                    }
                }
            }

            // Save CTA Button
            Rectangle {
                Layout.fillWidth: true
                height: 42
                radius: 9
                color: saveMouse.pressed ? "#15BD68" : (saveMouse.containsMouse ? "#25F088" : "#1CE47E")

                Behavior on color { ColorAnimation { duration: 110 } }

                Text {
                    anchors.centerIn: parent
                    text: "Сохранить"
                    color: "#041B0E"
                    font.family: "Inter"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                }

                MouseArea {
                    id: saveMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.doSave()
                }
            }
        }
    }
}
