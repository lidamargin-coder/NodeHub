import QtQuick
import QtQuick.Layouts
import QtQuick.Effects

Item {
    id: root
    property string modId: ""
    property string title: ""
    property string author: ""
    property string version: ""
    property string gameVersion: ""
    property string thumbnailUrl: ""
    property bool selected: false

    signal clicked()

    readonly property string displayVersion: {
        if (root.gameVersion && root.gameVersion.length > 0) {
            return root.gameVersion.startsWith("v") || root.gameVersion.startsWith("V")
                   ? root.gameVersion
                   : ("v" + root.gameVersion);
        }
        if (root.version && root.version.length > 0) {
            return root.version.startsWith("v") || root.version.startsWith("V")
                   ? root.version
                   : ("v" + root.version);
        }
        return "";
    }

    property var dlStatus: null

    function updateCardDl() {
        dlStatus = downloadManager.getModStatus(root.modId);
    }

    Component.onCompleted: updateCardDl()
    onModIdChanged: updateCardDl()

    Connections {
        target: downloadManager
        function onModStatusChanged(id) {
            if (id === root.modId) {
                root.updateCardDl();
            }
        }
    }

    readonly property bool isDl: dlStatus && (dlStatus.isDownloading || dlStatus.isQueued)
    readonly property bool isDone: (dlStatus && dlStatus.isFinished) || app.isModInstalled(root.modId, root.title)

    // Subtle outer emerald glow when selected or downloading
    Rectangle {
        anchors.fill: cardBody
        anchors.margins: -3
        radius: 13
        color: "transparent"
        border.width: 2
        border.color: root.selected ? "#661CE47E" : (root.isDl ? "#331CE47E" : "transparent")
        visible: root.selected || root.isDl
        opacity: root.selected ? 1.0 : 0.6
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    Rectangle {
        id: cardBody
        anchors.fill: parent
        anchors.margins: 6
        radius: 10
        color: root.selected ? "#15261E" : (cardMouse.containsMouse ? "#15201C" : "#111815")
        border.width: root.selected ? 2 : 1
        border.color: root.selected ? "#1CE47E" : (cardMouse.containsMouse ? "#2B4035" : "#1B2721")

        scale: cardMouse.pressed ? 0.98 : (cardMouse.containsMouse ? 1.015 : 1.0)
        Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        Behavior on color { ColorAnimation { duration: 130 } }
        Behavior on border.color { ColorAnimation { duration: 130 } }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            // Thumbnail with rounded mask and smooth zoom
            Item {
                id: imgBox
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                Rectangle {
                    anchors.fill: parent
                    radius: 7
                    color: "#0C110F"
                }

                Image {
                    id: thumbImg
                    anchors.fill: parent
                    source: root.thumbnailUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    cache: true
                    visible: false
                    smooth: true

                    scale: cardMouse.containsMouse ? 1.06 : 1.0
                    Behavior on scale { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
                }

                Rectangle {
                    id: imgMask
                    anchors.fill: parent
                    radius: 7
                    color: "white"
                    visible: false
                    layer.enabled: true
                }

                MultiEffect {
                    anchors.fill: thumbImg
                    source: thumbImg
                    maskEnabled: true
                    maskSource: imgMask
                    visible: thumbImg.status === Image.Ready
                }

                // Loading skeleton pulse
                Rectangle {
                    anchors.fill: parent
                    radius: 7
                    color: "#17221D"
                    visible: thumbImg.status !== Image.Ready
                }

                // Game Version badge in top-right of thumbnail (BeamNG Game Version)
                Rectangle {
                    visible: root.displayVersion.length > 0
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 6
                    height: 20
                    width: verTxt.implicitWidth + 12
                    radius: 5
                    color: "#D90B100E"
                    border.width: 1
                    border.color: "#331CE47E"

                    Text {
                        id: verTxt
                        anchors.centerIn: parent
                        text: root.displayVersion
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                // Active Download Progress Bar on Card
                Rectangle {
                    visible: root.isDl
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 4
                    color: "#080F0B"
                    radius: 2
                    clip: true

                    Rectangle {
                        height: parent.height
                        width: parent.width * Math.min(1.0, Math.max(0.05, root.dlStatus.progress || 0.0))
                        radius: 2
                        color: "#1CE47E"
                        Behavior on width { NumberAnimation { duration: 100 } }
                    }
                }

                // Installed Badge
                Rectangle {
                    visible: root.isDone && !root.isDl
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 6
                    height: 18
                    width: doneTxt.implicitWidth + 10
                    radius: 4
                    color: "#183B28"
                    border.width: 1
                    border.color: "#1CE47E"

                    Text {
                        id: doneTxt
                        anchors.centerIn: parent
                        text: "Установлен ✓"
                        color: "#1CE47E"
                        font.family: "Inter"
                        font.pixelSize: 9
                        font.weight: Font.DemiBold
                    }
                }
            }

            // Title & Author
            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 2
                Layout.rightMargin: 2
                Layout.bottomMargin: 2
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: root.title
                    color: root.selected ? "#FFFFFF" : "#E3ECE7"
                    font.family: "Inter"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }

                Text {
                    Layout.fillWidth: true
                    text: root.author
                    color: "#73887E"
                    font.family: "Inter"
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }
            }
        }

        MouseArea {
            id: cardMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked()
        }
    }
}
