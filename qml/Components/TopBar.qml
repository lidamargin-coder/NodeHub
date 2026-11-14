import QtQuick
import QtQuick.Layouts

Item {
    id: root
    height: 56
    implicitHeight: 56
    Layout.preferredHeight: 56
    Layout.fillWidth: true

    property alias searchText: searchInput.text
    property var targetWindow: null
    property bool downloadsOpen: false

    signal searchSubmitted(string query)
    signal settingsClicked()
    signal minimizeClicked()
    signal closeClicked()
    signal toggleDownloads()

    // Window drag area behind controls
    MouseArea {
        anchors.fill: parent
        onPressed: {
            if (root.targetWindow)
                root.targetWindow.startSystemMove()
        }
    }

    // Bottom subtle divider
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#16231D"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 14
        spacing: 12

        // Search input box
        Rectangle {
            Layout.preferredWidth: 260
            Layout.maximumWidth: 320
            Layout.fillWidth: true
            implicitHeight: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignVCenter
            radius: 9
            color: searchInput.activeFocus ? "#15201B" : "#111815"
            border.width: 1
            border.color: searchInput.activeFocus ? "#1CE47E" : "#1E2B25"

            Behavior on border.color { ColorAnimation { duration: 130 } }
            Behavior on color { ColorAnimation { duration: 130 } }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 10
                spacing: 8

                // Magnifier icon
                Canvas {
                    width: 14
                    height: 14
                    Layout.alignment: Qt.AlignVCenter
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.strokeStyle = "#6B8075";
                        ctx.lineWidth = 1.6;
                        ctx.lineCap = "round";
                        ctx.beginPath();
                        ctx.arc(6.0, 6.0, 4.2, 0, Math.PI * 2);
                        ctx.moveTo(9.2, 9.2);
                        ctx.lineTo(12.5, 12.5);
                        ctx.stroke();
                    }
                }

                TextInput {
                    id: searchInput
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    color: "#E8F0EC"
                    selectionColor: "#1CE47E"
                    selectedTextColor: "#061A10"
                    font.family: "Inter"
                    font.pixelSize: 13
                    clip: true

                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        text: "Поиск модов..."
                        color: "#5C7066"
                        font: searchInput.font
                        visible: !searchInput.text && !searchInput.activeFocus
                    }

                    onAccepted: root.searchSubmitted(text)
                }

                // Clear search button
                Rectangle {
                    visible: searchInput.text.length > 0
                    width: 20
                    height: 20
                    radius: 10
                    color: clearMouse.containsMouse ? "#23352D" : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: "#8BA196"
                        font.pixelSize: 14
                    }

                    MouseArea {
                        id: clearMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchInput.text = "";
                            root.searchSubmitted("");
                        }
                    }
                }
            }
        }

        // Sorting Segmented Control (Newest, Popular, Downloads)
        Rectangle {
            implicitWidth: sortRow.implicitWidth + 8
            Layout.preferredWidth: implicitWidth
            implicitHeight: 34
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignVCenter
            radius: 8
            color: "#0E1612"
            border.width: 1
            border.color: "#1C2A23"

            Row {
                id: sortRow
                anchors.centerIn: parent
                spacing: 2

                Repeater {
                    model: [
                        { slug: "newest", label: "Новинки" },
                        { slug: "popularity", label: "Популярные" },
                        { slug: "downloads", label: "Скачивания" }
                    ]

                    delegate: Rectangle {
                        width: tabTxt.implicitWidth + 16
                        height: 26
                        radius: 6

                        readonly property bool isCurrent: app.currentSort === modelData.slug
                        readonly property bool isHov: sortBtnMouse.containsMouse

                        color: isCurrent ? "#1E3C2B" : (isHov ? "#14201A" : "transparent")
                        border.width: isCurrent ? 1 : 0
                        border.color: isCurrent ? "#296245" : "transparent"

                        Behavior on color { ColorAnimation { duration: 120 } }

                        Text {
                            id: tabTxt
                            anchors.centerIn: parent
                            text: modelData.label
                            color: isCurrent ? "#1CE47E" : (isHov ? "#D1E2D9" : "#7C9187")
                            font.family: "Inter"
                            font.pixelSize: 11
                            font.weight: isCurrent ? Font.DemiBold : Font.Normal
                        }

                        MouseArea {
                            id: sortBtnMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: app.setSort(modelData.slug)
                        }
                    }
                }
            }
        }

        // Expander drag area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // Play BeamNG.drive Button with Tactical Animated Feedback
        Rectangle {
            id: playBtn
            property bool isLaunching: false

            implicitWidth: Math.max(94, playRow.implicitWidth + 24)
            Layout.preferredWidth: implicitWidth
            implicitHeight: 34
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignVCenter
            radius: 8
            color: isLaunching ? "#15C86C" : (playMouse.pressed ? "#148F50" : (playMouse.containsMouse ? "#20E883" : "#1CE47E"))
            scale: playMouse.pressed ? 0.96 : (isLaunching ? 1.02 : (playMouse.containsMouse ? 1.02 : 1.0))

            Behavior on scale { NumberAnimation { duration: 90 } }
            Behavior on color { ColorAnimation { duration: 110 } }
            Behavior on Layout.preferredWidth { NumberAnimation { duration: 120 } }

            // Subtle pulsing aura while launching
            Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#1CE47E"
                opacity: playBtn.isLaunching ? 0.4 : 0.0
                scale: playBtn.isLaunching ? 1.15 : 1.0
                z: -1

                SequentialAnimation on opacity {
                    running: playBtn.isLaunching
                    loops: Animation.Infinite
                    NumberAnimation { from: 0.2; to: 0.55; duration: 550; easing.type: Easing.InOutQuad }
                    NumberAnimation { from: 0.55; to: 0.2; duration: 550; easing.type: Easing.InOutQuad }
                }
            }

            Row {
                id: playRow
                anchors.centerIn: parent
                spacing: 7

                Text {
                    id: playIcon
                    text: playBtn.isLaunching ? "⟳" : "▶"
                    color: "#031C0E"
                    font.pixelSize: playBtn.isLaunching ? 14 : 11
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter

                    RotationAnimator on rotation {
                        running: playBtn.isLaunching
                        loops: Animation.Infinite
                        from: 0
                        to: 360
                        duration: 850
                    }
                }

                Text {
                    text: playBtn.isLaunching ? "Запуск..." : "Играть"
                    color: "#031C0E"
                    font.family: "Inter"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Timer {
                id: launchTimer
                interval: 2400
                onTriggered: playBtn.isLaunching = false
            }

            MouseArea {
                id: playMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: playBtn.isLaunching ? Qt.ArrowCursor : Qt.PointingHandCursor
                onClicked: {
                    if (playBtn.isLaunching) return;
                    playBtn.isLaunching = true;
                    launchTimer.restart();
                    app.launchGame();
                }
            }
        }

        // Active Downloads Button / Badge
        Rectangle {
            id: dlBtn
            implicitWidth: dlBtnRow.implicitWidth + 22
            Layout.preferredWidth: implicitWidth
            implicitHeight: 34
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignVCenter
            radius: 8
            color: root.downloadsOpen ? "#1C3627" : (dlBtnMouse.containsMouse ? "#18261F" : "#111815")
            border.width: 1
            border.color: downloadManager.activeCount > 0 ? "#1CE47E" : (root.downloadsOpen ? "#264835" : "#1D2B24")

            Behavior on color { ColorAnimation { duration: 120 } }
            Behavior on border.color { ColorAnimation { duration: 120 } }
            Behavior on Layout.preferredWidth { NumberAnimation { duration: 120 } }

            Row {
                id: dlBtnRow
                anchors.centerIn: parent
                spacing: 7

                // Download icon
                Canvas {
                    width: 13
                    height: 13
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.strokeStyle = downloadManager.activeCount > 0 ? "#1CE47E" : "#8FA399";
                        ctx.lineWidth = 1.6;
                        ctx.lineCap = "round";
                        ctx.lineJoin = "round";

                        // Arrow down
                        ctx.beginPath();
                        ctx.moveTo(6.5, 1.5);
                        ctx.lineTo(6.5, 8.5);
                        ctx.moveTo(3.5, 6.0);
                        ctx.lineTo(6.5, 9.0);
                        ctx.lineTo(9.5, 6.0);
                        ctx.stroke();

                        // Base tray
                        ctx.beginPath();
                        ctx.moveTo(1.5, 10.5);
                        ctx.lineTo(11.5, 10.5);
                        ctx.stroke();
                    }
                }

                Text {
                    text: "Загрузки"
                    color: downloadManager.activeCount > 0 ? "#FFFFFF" : "#8FA399"
                    font.family: "Inter"
                    font.pixelSize: 12
                    font.weight: downloadManager.activeCount > 0 ? Font.DemiBold : Font.Normal
                }

                // Active count badge with pulse
                Rectangle {
                    visible: downloadManager.activeCount > 0
                    width: 18
                    height: 18
                    radius: 9
                    color: "#1CE47E"

                    Text {
                        anchors.centerIn: parent
                        text: String(downloadManager.activeCount)
                        color: "#03170D"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }
            }

            MouseArea {
                id: dlBtnMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.toggleDownloads()
            }
        }

        // Settings gear button
        Rectangle {
            implicitWidth: 34
            Layout.preferredWidth: 34
            implicitHeight: 34
            Layout.preferredHeight: 34
            Layout.alignment: Qt.AlignVCenter
            radius: 8
            color: gearMouse.containsMouse ? "#1A2621" : "#111815"
            border.width: 1
            border.color: gearMouse.containsMouse ? "#2B3D35" : "#1D2A24"

            Behavior on color { ColorAnimation { duration: 120 } }

            Canvas {
                anchors.centerIn: parent
                width: 16
                height: 16
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.reset();
                    ctx.strokeStyle = gearMouse.containsMouse ? "#1CE47E" : "#8FA399";
                    ctx.lineWidth = 1.5;
                    ctx.lineCap = "round";
                    var cx = 8, cy = 8;
                    ctx.beginPath();
                    ctx.arc(cx, cy, 2.6, 0, Math.PI * 2);
                    ctx.stroke();
                    for (var i = 0; i < 8; i++) {
                        var a = i * Math.PI / 4;
                        ctx.beginPath();
                        ctx.moveTo(cx + Math.cos(a) * 4.5, cy + Math.sin(a) * 4.5);
                        ctx.lineTo(cx + Math.cos(a) * 6.4, cy + Math.sin(a) * 6.4);
                        ctx.stroke();
                    }
                    ctx.beginPath();
                    ctx.arc(cx, cy, 5.0, 0, Math.PI * 2);
                    ctx.stroke();
                }
                property bool hovered: gearMouse.containsMouse
                onHoveredChanged: requestPaint()
            }

            MouseArea {
                id: gearMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.settingsClicked()
            }
        }

        // Vertical divider
        Rectangle {
            implicitWidth: 1
            Layout.preferredWidth: 1
            implicitHeight: 18
            Layout.preferredHeight: 18
            color: "#1E2B25"
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 2
            Layout.rightMargin: 4
        }

        // Window Controls (Minimize and Close ONLY - Maximize is removed since window size is locked)
        Row {
            id: winControlsRow
            spacing: 4
            Layout.preferredWidth: 68
            Layout.alignment: Qt.AlignVCenter

            // Minimize
            Rectangle {
                width: 32
                height: 32
                radius: 7
                color: minMouse.containsMouse ? "#1A2520" : "transparent"

                Behavior on color { ColorAnimation { duration: 100 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 10
                    height: 1.5
                    radius: 1
                    color: minMouse.containsMouse ? "#FFFFFF" : "#8FA399"
                }

                MouseArea {
                    id: minMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.minimizeClicked()
                }
            }

            // Close
            Rectangle {
                width: 34
                height: 32
                radius: 7
                color: closeMouse.containsMouse ? "#C93B3B" : "transparent"

                Behavior on color { ColorAnimation { duration: 100 } }

                Canvas {
                    anchors.centerIn: parent
                    width: 10
                    height: 10
                    property bool hovered: closeMouse.containsMouse
                    onHoveredChanged: requestPaint()
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.strokeStyle = closeMouse.containsMouse ? "#FFFFFF" : "#8FA399";
                        ctx.lineWidth = 1.6;
                        ctx.lineCap = "round";
                        ctx.beginPath();
                        ctx.moveTo(1, 1); ctx.lineTo(9, 9);
                        ctx.moveTo(9, 1); ctx.lineTo(1, 9);
                        ctx.stroke();
                    }
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.closeClicked()
                }
            }
        }
    }
}
