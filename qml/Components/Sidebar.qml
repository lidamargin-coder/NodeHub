import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    width: 224
    implicitWidth: 224
    color: "#0C1210"

    property string currentCategory: "cars"
    property var targetWindow: null
    signal categorySelected(string slug)

    // Subtle right divider
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: "#1B2621"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 14
        anchors.bottomMargin: 18
        spacing: 18

        // Brand title with minimalist logo and smooth window drag
        Item {
            Layout.fillWidth: true
            height: 40

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    if (root.targetWindow)
                        root.targetWindow.startSystemMove()
                }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 4
                spacing: 10

                Image {
                    width: 26
                    height: 26
                    sourceSize.width: 26
                    sourceSize.height: 26
                    source: "qrc:/NodeHub/logos/logo3_hex_core.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "NodeHub"
                    color: "#1CE47E"
                    font.family: "Inter"
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    font.letterSpacing: -0.4
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Category List
        ListView {
            id: catList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            interactive: false

            model: ListModel {
                ListElement { slug: "cars";      label: "Легковые авто"; iconType: "car" }
                ListElement { slug: "jeeps";     label: "Внедорожники";  iconType: "jeep" }
                ListElement { slug: "trucks";    label: "Грузовики";     iconType: "truck" }
                ListElement { slug: "maps";      label: "Карты";         iconType: "map" }
                ListElement { slug: "racing";    label: "Спорт";         iconType: "sport" }
                ListElement { slug: "others";    label: "Прочее";        iconType: "grid" }
                ListElement { slug: "all";       label: "Все моды";      iconType: "all" }
                ListElement { slug: "installed"; label: "Установленные"; iconType: "folder" }
            }

            delegate: Rectangle {
                id: catItem
                width: catList.width
                height: 40
                radius: 9

                readonly property bool isActive: root.currentCategory === model.slug
                readonly property bool isHovered: mouseArea.containsMouse

                color: isActive ? "#153626" : (isHovered ? "#131C18" : "transparent")
                border.width: isActive ? 1 : 0
                border.color: isActive ? "#245C40" : "transparent"

                Behavior on color { ColorAnimation { duration: 130 } }

                // Left active indicator pill
                Rectangle {
                    visible: catItem.isActive
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    width: 3
                    height: 18
                    radius: 2
                    color: "#1CE47E"
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: catItem.isHovered && !catItem.isActive ? 18 : 14
                    anchors.rightMargin: 12
                    spacing: 11

                    Behavior on anchors.leftMargin { NumberAnimation { duration: 120; easing.type: Easing.OutQuad } }

                    Canvas {
                        id: iconCanvas
                        width: 17
                        height: 17
                        Layout.alignment: Qt.AlignVCenter

                        property color strokeColor: catItem.isActive ? "#1CE47E" : (catItem.isHovered ? "#D0DCD6" : "#7A8C84")
                        onStrokeColorChanged: requestPaint()

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = strokeColor;
                            ctx.fillStyle = strokeColor;
                            ctx.lineWidth = 1.6;
                            ctx.lineCap = "round";
                            ctx.lineJoin = "round";

                            var t = model.iconType;
                            if (t === "car") {
                                ctx.beginPath();
                                ctx.moveTo(2.5, 11.5);
                                ctx.lineTo(3.8, 7.2);
                                ctx.lineTo(6.0, 5.2);
                                ctx.lineTo(11.2, 5.2);
                                ctx.lineTo(13.4, 7.5);
                                ctx.lineTo(15.0, 8.2);
                                ctx.lineTo(15.0, 11.5);
                                ctx.closePath();
                                ctx.stroke();
                                ctx.beginPath(); ctx.arc(5.2, 12.2, 1.5, 0, Math.PI * 2); ctx.fill();
                                ctx.beginPath(); ctx.arc(12.2, 12.2, 1.5, 0, Math.PI * 2); ctx.fill();
                            } else if (t === "jeep") {
                                ctx.beginPath();
                                ctx.rect(2.5, 6.0, 12.0, 5.5);
                                ctx.moveTo(4.0, 6.0); ctx.lineTo(5.0, 3.8); ctx.lineTo(11.5, 3.8); ctx.lineTo(12.5, 6.0);
                                ctx.stroke();
                                ctx.beginPath(); ctx.arc(5.0, 12.8, 1.7, 0, Math.PI * 2); ctx.fill();
                                ctx.beginPath(); ctx.arc(12.0, 12.8, 1.7, 0, Math.PI * 2); ctx.fill();
                            } else if (t === "truck") {
                                ctx.beginPath();
                                ctx.rect(1.8, 4.5, 8.5, 7.0);
                                ctx.moveTo(10.3, 6.5); ctx.lineTo(13.5, 6.5); ctx.lineTo(15.2, 8.8); ctx.lineTo(15.2, 11.5); ctx.lineTo(10.3, 11.5);
                                ctx.stroke();
                                ctx.beginPath(); ctx.arc(4.8, 12.6, 1.6, 0, Math.PI * 2); ctx.fill();
                                ctx.beginPath(); ctx.arc(12.8, 12.6, 1.6, 0, Math.PI * 2); ctx.fill();
                            } else if (t === "map") {
                                ctx.beginPath();
                                ctx.moveTo(2.5, 4.5); ctx.lineTo(6.5, 3.2); ctx.lineTo(10.5, 4.8); ctx.lineTo(14.5, 3.5);
                                ctx.lineTo(14.5, 12.5); ctx.lineTo(10.5, 13.8); ctx.lineTo(6.5, 12.2); ctx.lineTo(2.5, 13.5);
                                ctx.closePath();
                                ctx.moveTo(6.5, 3.2); ctx.lineTo(6.5, 12.2);
                                ctx.moveTo(10.5, 4.8); ctx.lineTo(10.5, 13.8);
                                ctx.stroke();
                            } else if (t === "sport") {
                                ctx.beginPath();
                                ctx.arc(8.5, 9.5, 6.0, Math.PI * 0.85, Math.PI * 2.15);
                                ctx.moveTo(8.5, 9.5); ctx.lineTo(12.2, 6.2);
                                ctx.stroke();
                                ctx.beginPath(); ctx.arc(8.5, 9.5, 1.3, 0, Math.PI * 2); ctx.fill();
                            } else if (t === "grid") {
                                ctx.beginPath();
                                ctx.roundedRect(2.5, 2.5, 5.0, 5.0, 1.2, 1.2);
                                ctx.roundedRect(9.5, 2.5, 5.0, 5.0, 1.2, 1.2);
                                ctx.roundedRect(2.5, 9.5, 5.0, 5.0, 1.2, 1.2);
                                ctx.roundedRect(9.5, 9.5, 5.0, 5.0, 1.2, 1.2);
                                ctx.stroke();
                            } else if (t === "folder") {
                                ctx.beginPath();
                                ctx.moveTo(2.5, 4.0);
                                ctx.lineTo(6.5, 4.0);
                                ctx.lineTo(8.0, 5.8);
                                ctx.lineTo(14.5, 5.8);
                                ctx.lineTo(14.5, 13.0);
                                ctx.lineTo(2.5, 13.0);
                                ctx.closePath();
                                ctx.stroke();
                            } else {
                                ctx.beginPath();
                                ctx.moveTo(3, 5); ctx.lineTo(14, 5);
                                ctx.moveTo(3, 8.5); ctx.lineTo(14, 8.5);
                                ctx.moveTo(3, 12); ctx.lineTo(14, 12);
                                ctx.stroke();
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: model.label
                        color: catItem.isActive ? "#FFFFFF" : (catItem.isHovered ? "#DDE8E2" : "#8FA399")
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: catItem.isActive ? Font.DemiBold : Font.Normal
                        elide: Text.ElideRight
                    }

                    // Installed count badge
                    Rectangle {
                        visible: model.slug === "installed" && installedMods.totalInstalledCount > 0
                        Layout.alignment: Qt.AlignVCenter
                        height: 18
                        width: countTxt.implicitWidth + 10
                        radius: 9
                        color: catItem.isActive ? "#1CE47E" : "#192820"
                        border.width: 1
                        border.color: catItem.isActive ? "#1CE47E" : "#22352B"

                        Text {
                            id: countTxt
                            anchors.centerIn: parent
                            text: String(installedMods.totalInstalledCount)
                            color: catItem.isActive ? "#041B0F" : "#1CE47E"
                            font.family: "JetBrains Mono"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.categorySelected(model.slug)
                }
            }
        }
    }
}
