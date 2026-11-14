import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root
    width: 380
    height: 420
    visible: opacity > 0.001
    opacity: isOpen ? 1.0 : 0.0
    property bool isOpen: false

    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.OutQuad } }

    // Close when clicking outside
    MouseArea {
        anchors.fill: parent
        onClicked: {} // consume clicks inside
    }

    Rectangle {
        anchors.fill: parent
        radius: 14
        color: "#0F1814"
        border.width: 1
        border.color: "#243B30"

        // Glow header accent
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "#1CE47E"
            opacity: 0.35
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 10

            // Header row
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Загрузки"
                    color: "#FFFFFF"
                    font.family: "Inter"
                    font.pixelSize: 15
                    font.weight: Font.Bold
                }

                Rectangle {
                    visible: downloadManager.activeCount > 0
                    width: countTxt.implicitWidth + 12
                    height: 20
                    radius: 10
                    color: "#163E2A"
                    border.width: 1
                    border.color: "#1CE47E"

                    Text {
                        id: countTxt
                        anchors.centerIn: parent
                        text: String(downloadManager.activeCount)
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                Item { Layout.fillWidth: true }

                Text {
                    visible: downloadManager.totalSpeedMBps > 0.05
                    text: downloadManager.totalSpeedMBps.toFixed(1) + " МБ/с"
                    color: "#1CE47E"
                    font.family: "JetBrains Mono"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }

                Rectangle {
                    visible: downloadManager.totalCount > 0
                    width: clearTxt.implicitWidth + 14
                    height: 24
                    radius: 6
                    color: clearMouse.containsMouse ? "#1A2B23" : "#131D18"
                    border.width: 1
                    border.color: "#203328"

                    Text {
                        id: clearTxt
                        anchors.centerIn: parent
                        text: "Очистить"
                        color: "#8FA399"
                        font.family: "Inter"
                        font.pixelSize: 11
                    }

                    MouseArea {
                        id: clearMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: downloadManager.clearFinished()
                    }
                }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#192820"
            }

            // Downloads List
            ListView {
                id: downloadList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                spacing: 8
                model: downloadManager

                delegate: Rectangle {
                    width: downloadList.width
                    height: 64
                    radius: 9
                    color: "#131C18"
                    border.width: 1
                    border.color: model.isDownloading ? "#264D39" : "#1C2B23"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 10

                        // Mini Thumbnail
                        Rectangle {
                            width: 48
                            height: 48
                            radius: 6
                            color: "#080D0B"
                            clip: true

                            Image {
                                anchors.fill: parent
                                source: model.thumbnailUrl
                                fillMode: Image.PreserveAspectCrop
                                visible: status === Image.Ready
                            }
                        }

                        // Info Column
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: model.title
                                    color: "#E8F0EC"
                                    font.family: "Inter"
                                    font.pixelSize: 12
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }

                                Text {
                                    visible: model.isDownloading && model.speedMBps > 0.05
                                    text: model.speedMBps.toFixed(1) + " МБ/с"
                                    color: "#1CE47E"
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Text {
                                    Layout.fillWidth: true
                                    text: model.statusText
                                    color: model.isFailed ? "#FF6B6B" : (model.isFinished ? "#1CE47E" : "#8FA399")
                                    font.family: "Inter"
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                }

                                Text {
                                    visible: model.isDownloading && model.bytesTotal > 0
                                    text: Math.round(model.progress * 100) + "%"
                                    color: "#1CE47E"
                                    font.family: "JetBrains Mono"
                                    font.pixelSize: 10
                                    font.weight: Font.Bold
                                }
                            }

                            // Mini Progress Track
                            Rectangle {
                                Layout.fillWidth: true
                                height: 4
                                radius: 2
                                color: "#0A100D"
                                clip: true
                                visible: model.isDownloading || model.isQueued

                                Rectangle {
                                    height: parent.height
                                    radius: 2
                                    width: model.isDownloading && model.bytesTotal > 0
                                           ? parent.width * Math.min(1.0, Math.max(0.0, model.progress))
                                           : (model.isQueued ? parent.width * 0.15 : 0)
                                    color: "#1CE47E"

                                    Behavior on width { NumberAnimation { duration: 120 } }
                                }
                            }
                        }

                        // Cancel / Remove button
                        Rectangle {
                            width: 24
                            height: 24
                            radius: 6
                            color: cancelMouse.containsMouse ? "#3A1E1E" : "#1A2520"

                            Text {
                                anchors.centerIn: parent
                                text: "×"
                                color: cancelMouse.containsMouse ? "#FF6B6B" : "#8FA399"
                                font.pixelSize: 14
                            }

                            MouseArea {
                                id: cancelMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: downloadManager.cancelDownload(model.modId)
                            }
                        }
                    }
                }

                // Empty state
                Item {
                    anchors.centerIn: parent
                    visible: downloadManager.totalCount === 0
                    width: 200
                    height: 80

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 6

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Нет активных загрузок"
                            color: "#6C8076"
                            font.family: "Inter"
                            font.pixelSize: 13
                            font.weight: Font.Medium
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "До 5 модов могут скачиваться одновременно"
                            color: "#465950"
                            font.family: "Inter"
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }
}
