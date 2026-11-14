import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Rectangle {
    id: root
    width: 372
    color: "#0E1512"

    property var detail: ({})
    property bool loading: false

    signal installClicked()
    signal cancelDownloadClicked()

    property int activeImageIndex: 0
    onDetailChanged: {
        activeImageIndex = 0;
        if (galleryList) galleryList.positionViewAtBeginning();
        if (fgHeroImg) fgHeroImg.opacity = 0.0;
    }

    onMainImageUrlChanged: {
        if (!mainImageUrl || mainImageUrl.length === 0) return;
        if (fgHeroImg && fgHeroImg.source.toString() !== mainImageUrl) {
            fgHeroImg.opacity = 0.0;
            fgHeroImg.source = mainImageUrl;
            if (bgHeroImg && bgHeroImg.source.toString() === "") {
                bgHeroImg.source = mainImageUrl;
            }
        }
    }

    function nextPhoto() {
        if (gallery && gallery.length > 1) {
            activeImageIndex = (activeImageIndex + 1) % gallery.length;
            syncGalleryPosition();
        }
    }

    function prevPhoto() {
        if (gallery && gallery.length > 1) {
            activeImageIndex = (activeImageIndex - 1 + gallery.length) % gallery.length;
            syncGalleryPosition();
        }
    }

    function syncGalleryPosition() {
        if (galleryList) {
            galleryList.positionViewAtIndex(activeImageIndex, ListView.Contain);
        }
    }

    readonly property var gallery: detail.galleryUrls ? detail.galleryUrls : []
    readonly property string mainImageUrl: gallery.length > activeImageIndex ? gallery[activeImageIndex] : ""
    readonly property bool hasMod: Boolean(detail && detail.title && detail.title.length > 0)

    // Current mod download status from downloadManager
    readonly property string modId: detail && detail.id ? detail.id : ""

    property bool isLocallyPending: false
    property var currentDl: null

    function updateDlStatus() {
        currentDl = downloadManager.getModStatus(root.modId);
        if (currentDl && (currentDl.isDownloading || currentDl.isQueued)) {
            isLocallyPending = true;
        } else if (currentDl && (currentDl.isFinished || currentDl.isFailed)) {
            isLocallyPending = false;
        }
    }

    onModIdChanged: {
        isLocallyPending = false;
        updateDlStatus();
    }

    Connections {
        target: downloadManager
        function onModStatusChanged(id) {
            if (id === root.modId) {
                root.updateDlStatus();
            }
        }
        function onActiveCountChanged() {
            root.updateDlStatus();
        }
        function onDownloadCompleted(id, title, path) {
            if (id === root.modId) {
                root.isLocallyPending = false;
                root.updateDlStatus();
            }
        }
        function onDownloadFailed(id, title, err) {
            if (id === root.modId) {
                root.isLocallyPending = false;
                root.updateDlStatus();
            }
        }
    }

    readonly property bool isDownloading: isLocallyPending ||
                                         (currentDl && (currentDl.isDownloading || currentDl.isQueued))
    readonly property bool isFinished: !isDownloading &&
                                       ((currentDl && currentDl.isFinished) ||
                                        (root.hasMod && app.isModInstalled(root.modId, root.detail.title)))

    // Left divider line
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 1
        color: "#1B2621"
    }

    // Empty state
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8
        visible: !root.hasMod && !root.loading

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Выберите мод"
            color: "#8FA399"
            font.family: "Inter"
            font.pixelSize: 15
            font.weight: Font.DemiBold
        }
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Нажмите на карточку слева для просмотра"
            color: "#576B61"
            font.family: "Inter"
            font.pixelSize: 12
        }
    }

    // Loading spinner
    BusyIndicator {
        anchors.centerIn: parent
        running: root.loading
        visible: root.loading
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.topMargin: 14
        anchors.bottomMargin: 22
        spacing: 12
        visible: root.hasMod && !root.loading

        // Main Screenshot + Thumbnail Carousel Strip
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            // Hero image container with smooth crossfading & rounded corners
            Item {
                id: heroContainer
                Layout.fillWidth: true
                height: 184

                // Dark frame background
                Rectangle {
                    anchors.fill: parent
                    radius: 10
                    color: "#080D0B"
                }

                // Masked image viewport
                Item {
                    id: heroViewport
                    anchors.fill: parent
                    layer.enabled: true
                    layer.effect: MultiEffect {
                        maskEnabled: true
                        maskSource: heroMask
                    }

                    // Background Image (holds previous photo so there is NEVER a black flash)
                    Image {
                        id: bgHeroImg
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                    }

                    // Foreground Image (loads new photo and crossfades in smoothly)
                    Image {
                        id: fgHeroImg
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                        opacity: 0.0

                        onStatusChanged: {
                            if (status === Image.Ready) {
                                crossfadeAnim.restart();
                            }
                        }
                    }

                    // Subtle vignette gradient for badge & arrow readability
                    Rectangle {
                        anchors.fill: parent
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#33000000" }
                            GradientStop { position: 0.3; color: "transparent" }
                            GradientStop { position: 0.7; color: "transparent" }
                            GradientStop { position: 1.0; color: "#55000000" }
                        }
                    }

                    NumberAnimation {
                        id: crossfadeAnim
                        target: fgHeroImg
                        property: "opacity"
                        from: 0.0
                        to: 1.0
                        duration: 180
                        easing.type: Easing.OutQuad
                        onFinished: {
                            bgHeroImg.source = fgHeroImg.source;
                        }
                    }
                }

                // Corner mask for hero viewport
                Rectangle {
                    id: heroMask
                    anchors.fill: parent
                    radius: 10
                    color: "white"
                    visible: false
                    layer.enabled: true
                }

                // Outer border outline
                Rectangle {
                    anchors.fill: parent
                    radius: 10
                    color: "transparent"
                    border.width: 1
                    border.color: "#223029"
                    z: 2
                }

                // Interactive MouseArea for navigation & wheel flipping
                MouseArea {
                    id: heroMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    z: 5
                    cursorShape: root.gallery.length > 1 ? Qt.PointingHandCursor : Qt.ArrowCursor

                    onWheel: (wheel) => {
                        if (root.gallery.length <= 1) return;
                        if (wheel.angleDelta.y < 0 || wheel.angleDelta.x > 0) {
                            root.nextPhoto();
                        } else if (wheel.angleDelta.y > 0 || wheel.angleDelta.x < 0) {
                            root.prevPhoto();
                        }
                    }

                    onClicked: (mouse) => {
                        if (root.gallery.length <= 1) return;
                        if (mouse.x < width / 3) {
                            root.prevPhoto();
                        } else {
                            root.nextPhoto();
                        }
                    }
                }

                // Photo Counter Badge (e.g. 1 / 6)
                Rectangle {
                    visible: root.gallery.length > 1
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 8
                    height: 20
                    width: counterTxt.implicitWidth + 12
                    radius: 5
                    color: "#CC090F0C"
                    border.width: 1
                    border.color: "#331CE47E"
                    z: 10

                    Text {
                        id: counterTxt
                        anchors.centerIn: parent
                        text: (root.activeImageIndex + 1) + " / " + root.gallery.length
                        color: "#1CE47E"
                        font.family: "JetBrains Mono"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }
                }

                // Left Hero Arrow Button
                Rectangle {
                    id: leftArrowBtn
                    visible: root.gallery.length > 1
                    opacity: (heroMouse.containsMouse || leftArrowMouse.containsMouse) ? 1.0 : 0.0
                    scale: leftArrowMouse.pressed ? 0.90 : (leftArrowMouse.containsMouse ? 1.08 : 1.0)
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    width: 30
                    height: 30
                    radius: 15
                    color: leftArrowMouse.pressed ? "#1CE47E" : (leftArrowMouse.containsMouse ? "#20382B" : "#CC0A120E")
                    border.width: 1
                    border.color: leftArrowMouse.containsMouse ? "#1CE47E" : "#2E473A"
                    z: 10

                    Behavior on opacity { NumberAnimation { duration: 140 } }
                    Behavior on scale { NumberAnimation { duration: 90 } }
                    Behavior on color { ColorAnimation { duration: 90 } }
                    Behavior on border.color { ColorAnimation { duration: 90 } }

                    Text {
                        anchors.centerIn: parent
                        text: "‹"
                        color: leftArrowMouse.pressed ? "#041B0E" : "#FFFFFF"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: leftArrowMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.prevPhoto()
                    }
                }

                // Right Hero Arrow Button
                Rectangle {
                    id: rightArrowBtn
                    visible: root.gallery.length > 1
                    opacity: (heroMouse.containsMouse || rightArrowMouse.containsMouse) ? 1.0 : 0.0
                    scale: rightArrowMouse.pressed ? 0.90 : (rightArrowMouse.containsMouse ? 1.08 : 1.0)
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    width: 30
                    height: 30
                    radius: 15
                    color: rightArrowMouse.pressed ? "#1CE47E" : (rightArrowMouse.containsMouse ? "#20382B" : "#CC0A120E")
                    border.width: 1
                    border.color: rightArrowMouse.containsMouse ? "#1CE47E" : "#2E473A"
                    z: 10

                    Behavior on opacity { NumberAnimation { duration: 140 } }
                    Behavior on scale { NumberAnimation { duration: 90 } }
                    Behavior on color { ColorAnimation { duration: 90 } }
                    Behavior on border.color { ColorAnimation { duration: 90 } }

                    Text {
                        anchors.centerIn: parent
                        text: "›"
                        color: rightArrowMouse.pressed ? "#041B0E" : "#FFFFFF"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    MouseArea {
                        id: rightArrowMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.nextPhoto()
                    }
                }
            }

            // Thumbnail Strip with Smooth Scrolling & Chevrons (> 4 photos support)
            RowLayout {
                Layout.fillWidth: true
                spacing: 4
                visible: root.gallery.length > 1

                // Left Scroll Chevron Button
                Rectangle {
                    visible: root.gallery.length > 4
                    width: 22
                    height: 44
                    radius: 5
                    color: prevMouse.containsMouse ? "#1E3327" : "#121A15"
                    border.width: 1
                    border.color: prevMouse.containsMouse ? "#2A4535" : "#1E2C24"

                    Text {
                        anchors.centerIn: parent
                        text: "‹"
                        color: prevMouse.containsMouse ? "#1CE47E" : "#9DB0A6"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    MouseArea {
                        id: prevMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            galleryList.contentX = Math.max(0, galleryList.contentX - 160);
                        }
                    }
                }

                // Horizontal ListView for all thumbnails
                ListView {
                    id: galleryList
                    Layout.fillWidth: true
                    height: 44
                    orientation: ListView.Horizontal
                    spacing: 6
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds
                    model: root.gallery

                    Behavior on contentX { NumberAnimation { duration: 160; easing.type: Easing.OutQuad } }

                    delegate: Rectangle {
                        id: thumbItem
                        width: 68
                        height: 44
                        radius: 6
                        color: "#0B100E"
                        clip: true

                        readonly property bool isSelected: root.activeImageIndex === index

                        border.width: isSelected ? 2 : 1
                        border.color: isSelected ? "#1CE47E" : (thumbMouse.containsMouse ? "#314D3E" : "#1D2822")
                        scale: thumbMouse.pressed ? 0.94 : (thumbMouse.containsMouse ? 1.04 : 1.0)

                        Behavior on scale { NumberAnimation { duration: 90 } }
                        Behavior on border.color { ColorAnimation { duration: 110 } }

                        Image {
                            anchors.fill: parent
                            anchors.margins: thumbItem.isSelected ? 2 : 1
                            source: modelData
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: true
                            opacity: thumbItem.isSelected ? 1.0 : (thumbMouse.containsMouse ? 0.9 : 0.65)
                            Behavior on opacity { NumberAnimation { duration: 120 } }
                        }

                        MouseArea {
                            id: thumbMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.activeImageIndex = index;
                                root.syncGalleryPosition();
                            }
                        }
                    }
                }

                // Right Scroll Chevron Button
                Rectangle {
                    visible: root.gallery.length > 4
                    width: 22
                    height: 44
                    radius: 5
                    color: nextMouse.containsMouse ? "#1E3327" : "#121A15"
                    border.width: 1
                    border.color: nextMouse.containsMouse ? "#2A4535" : "#1E2C24"

                    Text {
                        anchors.centerIn: parent
                        text: "›"
                        color: nextMouse.containsMouse ? "#1CE47E" : "#9DB0A6"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    MouseArea {
                        id: nextMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            galleryList.contentX = Math.min(galleryList.contentWidth - galleryList.width, galleryList.contentX + 160);
                        }
                    }
                }
            }
        }

        // Title & Metadata Pills
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: root.detail.title || ""
                color: "#FFFFFF"
                font.family: "Inter"
                font.pixelSize: 17
                font.weight: Font.Bold
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }

            Flow {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: {
                        var pills = [];
                        if (root.detail.version)     pills.push({ label: "Версия " + root.detail.version, mono: true });
                        if (root.detail.gameVersion) pills.push({ label: "BeamNG " + root.detail.gameVersion, mono: true });
                        if (root.detail.author)      pills.push({ label: "Автор: " + root.detail.author, mono: false });
                        if (root.detail.sizeText)    pills.push({ label: "Размер: " + root.detail.sizeText, mono: true });
                        return pills;
                    }

                    delegate: Rectangle {
                        height: 24
                        width: pillTxt.implicitWidth + 14
                        radius: 6
                        color: "#151F1B"
                        border.width: 1
                        border.color: "#24332C"

                        Text {
                            id: pillTxt
                            anchors.centerIn: parent
                            text: modelData.label
                            color: "#AFC2B9"
                            font.family: modelData.mono ? "JetBrains Mono" : "Inter"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }
                    }
                }
            }
        }

        // Scrollable Description, Features, Changelog
        ScrollView {
            id: descScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ScrollBar.vertical: ScrollBar {
                id: descScrollBar
                policy: ScrollBar.AsNeeded
                width: 5
                anchors.right: parent.right

                contentItem: Rectangle {
                    implicitWidth: 5
                    radius: 3
                    color: descScrollBar.pressed ? "#1CE47E" : (descScrollBar.hovered ? "#355444" : "#1F2E27")
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
            }

            ColumnLayout {
                width: Math.max(100, descScrollView.availableWidth - 8)
                spacing: 12

                // Description
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: (root.detail.description || "").length > 0

                    Text {
                        text: "Описание"
                        color: "#E8F0EC"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.preferredWidth: Math.max(100, descScrollView.availableWidth - 10)
                        text: root.detail.description || ""
                        color: "#9DB0A6"
                        font.family: "Inter"
                        font.pixelSize: 12
                        lineHeight: 1.35
                        wrapMode: Text.Wrap
                    }
                }

                // Features
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: Boolean(root.detail.features && root.detail.features.length > 0)

                    Text {
                        text: "Особенности"
                        color: "#E8F0EC"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }

                    Repeater {
                        model: root.detail.features || []
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Math.max(100, descScrollView.availableWidth - 10)
                            spacing: 8

                            Rectangle {
                                Layout.alignment: Qt.AlignTop
                                Layout.topMargin: 6
                                width: 5
                                height: 5
                                radius: 3
                                color: "#1CE47E"
                            }

                            Text {
                                Layout.fillWidth: true
                                text: modelData
                                color: "#9DB0A6"
                                font.family: "Inter"
                                font.pixelSize: 12
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }

                // Changelog
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    visible: (root.detail.changelogText || "").length > 0

                    Text {
                        text: "История обновлений"
                        color: "#E8F0EC"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: logCol.implicitHeight + 16
                        radius: 8
                        color: "#121A17"
                        border.width: 1
                        border.color: "#1E2B25"

                        ColumnLayout {
                            id: logCol
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                Layout.preferredWidth: Math.max(100, logCol.width)
                                text: root.detail.changelogTitle || ""
                                color: "#1CE47E"
                                font.family: "Inter"
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                                wrapMode: Text.Wrap
                                visible: text.length > 0
                            }

                            Text {
                                Layout.fillWidth: true
                                Layout.preferredWidth: Math.max(100, logCol.width)
                                text: root.detail.changelogText || ""
                                color: "#9DB0A6"
                                font.family: "Inter"
                                font.pixelSize: 11
                                lineHeight: 1.3
                                wrapMode: Text.Wrap
                            }
                        }
                    }
                }
            }
        }

        // Bottom Install / Progress / Finished CTA Button
        Item {
            Layout.fillWidth: true
            height: 46

            // 1. Downloading CTA Button State ("Загрузка...")
            Rectangle {
                anchors.fill: parent
                visible: root.isDownloading
                radius: 10
                color: "#11261B"
                border.width: 1
                border.color: "#1CE47E"
                clip: true

                // Background Progress Fill
                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: root.currentDl && root.currentDl.bytesTotal > 0
                           ? parent.width * Math.min(1.0, Math.max(0.05, root.currentDl.progress))
                           : parent.width * 0.12
                    color: "#1CE47E"
                    opacity: 0.22

                    Behavior on width { NumberAnimation { duration: 150 } }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 12
                    spacing: 8

                    // Rotating spinner
                    Text {
                        text: "⟳"
                        color: "#1CE47E"
                        font.pixelSize: 15
                        font.bold: true
                        Layout.alignment: Qt.AlignVCenter

                        RotationAnimator on rotation {
                            running: root.isDownloading
                            loops: Animation.Infinite
                            from: 0
                            to: 360
                            duration: 850
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: {
                            if (root.currentDl && root.currentDl.bytesTotal > 0 && root.currentDl.progress > 0) {
                                return "Загрузка... " + Math.round(root.currentDl.progress * 100) + "%" +
                                       (root.currentDl.speedMBps > 0 ? ("  (" + root.currentDl.speedMBps.toFixed(1) + " МБ/с)") : "");
                            }
                            return "Загрузка...";
                        }
                        color: "#1CE47E"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignVCenter
                    }

                    // Cancel button
                    Rectangle {
                        width: 26
                        height: 26
                        radius: 6
                        color: cancelDlMouse.containsMouse ? "#3A1B1B" : "#1B2A22"
                        border.width: 1
                        border.color: cancelDlMouse.containsMouse ? "#632727" : "#243D2F"
                        Layout.alignment: Qt.AlignVCenter

                        Text {
                            anchors.centerIn: parent
                            text: "✕"
                            color: cancelDlMouse.containsMouse ? "#FF6B6B" : "#8FA399"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: cancelDlMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.isLocallyPending = false;
                                downloadManager.cancelDownload(root.modId);
                            }
                        }
                    }
                }
            }

            // 2. Finished CTA Button
            Rectangle {
                anchors.fill: parent
                visible: !root.isDownloading && root.isFinished
                radius: 10
                color: "#163826"
                border.width: 1
                border.color: "#1CE47E"

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "Установлен ✓  |  Открыть папку"
                        color: "#1CE47E"
                        font.family: "Inter"
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: app.openTargetFolderForSelected()
                }
            }

            // 3. Install CTA Button (Default state)
            Rectangle {
                anchors.fill: parent
                visible: !root.isDownloading && !root.isFinished
                radius: 10
                color: installBtnMouse.pressed ? "#15BD68" : (installBtnMouse.containsMouse ? "#25F088" : "#1CE47E")

                scale: installBtnMouse.pressed ? 0.98 : 1.0
                Behavior on scale { NumberAnimation { duration: 100 } }
                Behavior on color { ColorAnimation { duration: 110 } }

                Text {
                    anchors.centerIn: parent
                    text: root.detail.sizeText
                          ? ("Установить мод (" + root.detail.sizeText + ")")
                          : "Установить мод"
                    color: "#041C0F"
                    font.family: "Inter"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                }

                MouseArea {
                    id: installBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.isLocallyPending = true;
                        root.installClicked();
                    }
                }
            }
        }
    }
}
