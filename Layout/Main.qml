import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    // window is the top-level application shell.
    id: window

    // width is the default desktop launch width.
    width: 1180

    // height is the default desktop launch height.
    height: 820

    // minimumWidth keeps the navigation and editor usable on narrow desktop windows.
    minimumWidth: 760

    // minimumHeight keeps the note page from collapsing vertically.
    minimumHeight: 560

    // visible shows the window immediately after QML loading completes.
    visible: true

    // title is displayed by the native window frame.
    title: "ABCNote"

    LayoutMirroring.enabled: i18n.rightToLeft
    LayoutMirroring.childrenInherit: true

    // color provides the neutral background behind paper-like note pages.
    color: "#f0f2f5"

    // header contains the application title and autosave status.
    header: ToolBar {
        // The toolbar background uses a quiet desktop-app neutral.
        background: Rectangle { color: "#f7f7f7" }

        RowLayout {
            // Fill the whole toolbar so status text can align to the right.
            anchors.fill: parent

            // spacing separates the title from status labels.
            spacing: 8

            Label {
                // The product name is the stable first-viewport brand signal.
                text: "ABCNote"

                // Bold title improves scanability in the toolbar.
                font.bold: true

                // font.pixelSize keeps toolbar typography compact.
                font.pixelSize: 18

                // leftPadding aligns the title with the navigation heading.
                leftPadding: 18

                // Layout.fillWidth pushes the status label to the right.
                Layout.fillWidth: true
            }

            Label {
                // statusMessage is updated by NoteController during open and save events.
                text: noteController.statusMessage

                // Muted color keeps status secondary to the app title.
                color: "#666666"

                // font.pixelSize matches compact toolbar text.
                font.pixelSize: 13

                // Layout.rightMargin keeps text away from the window edge.
                Layout.rightMargin: 4
            }

            Rectangle {
                // Global Markdown mode switch applies to every daily note.
                Layout.preferredWidth: 160
                Layout.preferredHeight: 34
                radius: 6
                color: "#f3f5f7"
                border.color: "#d7dde5"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 3

                    Button {
                        text: i18n.t("notes.edit")
                        checkable: true
                        checked: !appSettings.markdownPreviewMode
                        flat: true
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        background: Rectangle {
                            radius: 5
                            color: parent.checked ? "#ffffff" : "transparent"
                            border.color: parent.checked ? "#cdd7e6" : "transparent"
                        }

                        contentItem: Text {
                            text: parent.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: parent.checked ? "#1d4f91" : "#5f6975"
                            font.pixelSize: 12
                            font.bold: parent.checked
                        }

                        onClicked: appSettings.setMarkdownPreviewMode(false)
                    }

                    Button {
                        text: i18n.t("notes.preview")
                        checkable: true
                        checked: appSettings.markdownPreviewMode
                        flat: true
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        background: Rectangle {
                            radius: 5
                            color: parent.checked ? "#ffffff" : "transparent"
                            border.color: parent.checked ? "#cdd7e6" : "transparent"
                        }

                        contentItem: Text {
                            text: parent.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: parent.checked ? "#1d4f91" : "#5f6975"
                            font.pixelSize: 12
                            font.bold: parent.checked
                        }

                        onClicked: appSettings.setMarkdownPreviewMode(true)
                    }
                }
            }

            ToolButton {
                // The gear opens data-directory settings.
                id: settingsButton

                // A compact symbol keeps the toolbar action visually lightweight.
                text: "\u2699"

                // tooltip names the compact icon affordance.
                ToolTip.visible: hovered
                ToolTip.text: i18n.t("settings.title")

                onClicked: settingsPopup.open()
            }

            ToolButton {
                // The info button opens developer information.
                id: aboutButton

                // A compact i icon matches common About actions.
                text: "i"

                // Bold text keeps the single-letter icon legible.
                font.bold: true

                // tooltip names the compact icon affordance.
                ToolTip.visible: hovered
                ToolTip.text: i18n.t("about.title")

                // Layout.rightMargin keeps the button away from the window edge.
                Layout.rightMargin: 12

                onClicked: aboutPopup.open()
            }
        }
    }

    Popup {
        // aboutPopup displays developer information.
        id: aboutPopup

        // Position near the top-right About button.
        x: Math.max(12, window.width - width - 16)
        y: 8

        // Modal false keeps the app feeling like a desktop toolbar menu.
        modal: false

        // Close when users click away.
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // Fixed dimensions keep placeholder or later profile text readable.
        width: 460
        height: 300

        background: Rectangle {
            // White panel matches the settings popup.
            color: "#ffffff"
            border.color: "#d7d7d7"
            radius: 6
        }

        ColumnLayout {
            // Content fills the popup with comfortable padding.
            anchors.fill: parent
            anchors.margins: 16
            spacing: 10

            Label {
                // About section heading.
                text: i18n.t("about.title")
                font.pixelSize: 16
                font.bold: true
                color: "#222222"
                Layout.fillWidth: true
            }

            GridLayout {
                // About details show release and contact metadata.
                columns: 2
                columnSpacing: 12
                rowSpacing: 8
                Layout.fillWidth: true
                Layout.fillHeight: true

                Label {
                    text: i18n.t("about.version")
                    color: "#666666"
                    font.pixelSize: 13
                    font.bold: true
                }

                Label {
                    text: Qt.application.version
                    color: "#333333"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Label {
                    text: i18n.t("about.repository")
                    color: "#666666"
                    font.pixelSize: 13
                    font.bold: true
                }

                Text {
                    text: "https://github.com/takalard/ABCNote.git"
                    color: "#245fba"
                    font.pixelSize: 13
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }

                Label {
                    text: i18n.t("about.author")
                    color: "#666666"
                    font.pixelSize: 13
                    font.bold: true
                }

                Label {
                    text: "kamel.wang"
                    color: "#333333"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Label {
                    text: i18n.t("about.email")
                    color: "#666666"
                    font.pixelSize: 13
                    font.bold: true
                }

                Text {
                    text: "kamel.wang@gmail.com"
                    color: "#245fba"
                    font.pixelSize: 13
                    wrapMode: Text.WrapAnywhere
                    Layout.fillWidth: true
                }

                Label {
                    text: i18n.t("about.wechat")
                    color: "#666666"
                    font.pixelSize: 13
                    font.bold: true
                }

                Label {
                    text: "sikimu"
                    color: "#333333"
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                // Close action aligns to the right.
                Layout.fillWidth: true

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    // Closes the About popup.
                    text: i18n.t("about.close")

                    onClicked: aboutPopup.close()
                }
            }
        }
    }

    Popup {
        // settingsPopup contains the current data directory and directory change action.
        id: settingsPopup

        // Position near the top-right settings button.
        x: Math.max(12, window.width - width - 16)
        y: 8

        // Modal false keeps the app feeling like a desktop toolbar menu.
        modal: false

        // Close when users click away.
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // Fixed dimensions keep settings readable without resizing from long paths.
        width: 520
        height: 520

        background: Rectangle {
            // White panel matches Qt desktop popup conventions.
            color: "#ffffff"
            border.color: "#d7d7d7"
            radius: 6
        }

        ScrollView {
            // settingsScroll keeps the settings panel usable as sections grow.
            anchors.fill: parent
            anchors.margins: 16
            clip: true

            ColumnLayout {
                // Content fills the popup with comfortable padding.
                width: settingsPopup.width - 32
                spacing: 10

                Label {
                    // Settings section heading.
                    text: i18n.t("settings.title")
                    font.pixelSize: 16
                    font.bold: true
                    color: "#222222"
                    Layout.fillWidth: true
                }

                Label {
                    // Language selector label.
                    text: i18n.t("settings.language")
                    font.pixelSize: 13
                    font.bold: true
                    color: "#555555"
                    Layout.fillWidth: true
                }

                ComboBox {
                    // languageCombo switches the persisted UI language.
                    id: languageCombo
                    model: i18n.languages
                    textRole: "nativeName"
                    valueRole: "code"
                    currentIndex: i18n.languageIndex()
                    Layout.fillWidth: true

                    onActivated: i18n.setLanguage(currentValue)

                    Connections {
                        target: i18n

                        function onLanguageChanged() {
                            languageCombo.currentIndex = i18n.languageIndex()
                        }
                    }
                }

                Label {
                    // Current data root label.
                    text: i18n.t("settings.dataDirectory")
                    font.pixelSize: 13
                    font.bold: true
                    color: "#555555"
                    Layout.fillWidth: true
                }

                TextField {
                    // dataRootField shows the selected directory without allowing accidental inline edits.
                    id: dataRootField
                    text: appSettings.dataRoot
                    readOnly: true
                    selectByMouse: true
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                RowLayout {
                    // Actions align to the right like a compact settings menu.
                    Layout.fillWidth: true

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        // Opens the system folder picker.
                        text: i18n.t("settings.changeDirectory")

                        onClicked: dataDirectoryDialog.open()
                    }
                }

                Rectangle {
                    // Divider separates data and AI settings.
                    height: 1
                    color: "#eeeeee"
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    Layout.bottomMargin: 4
                }

                Label {
                    // AI summary settings section heading.
                    text: i18n.t("settings.aiSummary")
                    font.pixelSize: 13
                    font.bold: true
                    color: "#555555"
                    Layout.fillWidth: true
                }

                Switch {
                    // aiSummarySwitch controls whether refresh may call the configured model.
                    id: aiSummarySwitch
                    text: i18n.t("settings.enableAiSummary")
                    checked: appSettings.aiSummaryEnabled
                    Layout.fillWidth: true
                }

                Label {
                    text: "Base URL"
                    font.pixelSize: 12
                    color: "#666666"
                    Layout.fillWidth: true
                }

                TextField {
                    // aiBaseUrlField stores an OpenAI-compatible API root.
                    id: aiBaseUrlField
                    text: appSettings.aiBaseUrl
                    placeholderText: "https://api.example.com/v1"
                    selectByMouse: true
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Label {
                    text: "Model"
                    font.pixelSize: 12
                    color: "#666666"
                    Layout.fillWidth: true
                }

                TextField {
                    // aiModelField stores the user-selected model id.
                    id: aiModelField
                    text: appSettings.aiModel
                    placeholderText: "model-name"
                    selectByMouse: true
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Label {
                    text: "API Key"
                    font.pixelSize: 12
                    color: "#666666"
                    Layout.fillWidth: true
                }

                TextField {
                    // aiApiKeyField accepts a new key without displaying the stored secret.
                    id: aiApiKeyField
                    placeholderText: appSettings.hasAiApiKey ? i18n.t("settings.apiKeySavedPlaceholder") : i18n.t("settings.apiKeyNewPlaceholder")
                    echoMode: TextInput.Password
                    selectByMouse: true
                    font.pixelSize: 13
                    Layout.fillWidth: true
                }

                Label {
                    // settings status reports migration and AI settings results.
                    text: appSettings.statusMessage
                    color: "#666666"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                RowLayout {
                    // AI actions align to the right.
                    Layout.fillWidth: true

                    Label {
                        text: appSettings.hasAiApiKey ? i18n.t("settings.keySaved") : i18n.t("settings.keyNotSaved")
                        color: appSettings.hasAiApiKey ? "#2f6f44" : "#8a5a00"
                        font.pixelSize: 12
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: i18n.t("settings.clearKey")
                        enabled: appSettings.hasAiApiKey

                        onClicked: appSettings.clearAiApiKey()
                    }

                    Button {
                        text: i18n.t("settings.saveAiSettings")

                        onClicked: {
                            appSettings.saveAiSummarySettings(aiSummarySwitch.checked,
                                                              aiBaseUrlField.text,
                                                              aiModelField.text,
                                                              aiApiKeyField.text)
                            aiApiKeyField.text = ""
                        }
                    }
                }
            }
        }
    }

    FolderDialog {
        // dataDirectoryDialog lets the user choose a new note data directory.
        id: dataDirectoryDialog

        // title describes the directory picker purpose.
        title: i18n.t("dialog.selectDataDirectory")

        // currentFolder starts from the current configured data root.
        currentFolder: "file:///" + appSettings.dataRoot.replace(/\\/g, "/")

        onAccepted: {
            // selectedFolder is a URL; AppSettings converts it to a local path.
            appSettings.migrateAndSwitchDataRootUrl(selectedFolder)
        }
    }

    RowLayout {
        // The main layout splits navigation and content.
        anchors.fill: parent

        // spacing remains zero so the sidebar touches the content background cleanly.
        spacing: 0

        Rectangle {
            // Sidebar width adapts slightly for smaller desktop/mobile-like windows.
            Layout.preferredWidth: window.width < 820 ? 210 : 280

            // Sidebar fills the available application height below the toolbar.
            Layout.fillHeight: true

            // Sidebar color separates navigation from the writing surface.
            color: "#fbfbfb"

            // border.color creates a simple divider next to the note stream.
            border.color: "#dddddd"

            ColumnLayout {
                // The sidebar column contains the heading and the navigation list.
                anchors.fill: parent

                // spacing is zero so the list starts directly after the heading padding.
                spacing: 0

                Label {
                    // Localized heading for date navigation.
                    text: i18n.t("notes.dateNotes")

                    // Heading size is larger than row text but still compact.
                    font.pixelSize: 16

                    // Bold heading distinguishes the navigation section title.
                    font.bold: true

                    // Dark color gives the heading enough contrast.
                    color: "#222222"

                    // leftPadding aligns heading text with top-level navigation rows.
                    leftPadding: 18

                    // topPadding gives breathing room below the toolbar.
                    topPadding: 18

                    // bottomPadding separates heading from the first navigation row.
                    bottomPadding: 12

                    // Fill width so padding applies across the sidebar.
                    Layout.fillWidth: true
                }

                ListView {
                    // navView displays the flattened year/month/day model.
                    id: navView

                    // The list fills sidebar width.
                    Layout.fillWidth: true

                    // The list consumes remaining sidebar height.
                    Layout.fillHeight: true

                    // clip prevents rows from drawing outside the sidebar.
                    clip: true

                    // navModel is provided by main.cpp as a context property.
                    model: navModel

                    delegate: ItemDelegate {
                        // Each row spans the full navigation list width.
                        width: ListView.view.width

                        // Fixed row height avoids layout shifts between levels.
                        height: 38

                        // navText is prefixed with an expansion marker for year and month rows.
                        text: expandable ? ((expanded ? "\u25be " : "\u25b8 ") + model.navText) : model.navText

                        // leftPadding uses the level role to create the three-level tree indentation.
                        leftPadding: 18 + level * 18

                        // Top-level years are slightly larger than month/day rows.
                        font.pixelSize: level === 0 ? 15 : 14

                        // Top-level years are bold section headers.
                        font.bold: level === 0

                        // All rows are enabled: groups toggle expansion and days jump to dates.
                        enabled: true

                        background: Rectangle {
                            // hovered highlights clickable day rows without adding visual noise.
                            color: hovered ? "#edf4ff" : "transparent"
                        }

                        onClicked: {
                            // expandable rows are year/month groups that toggle their children.
                            if (expandable) {
                                navModel.toggleExpanded(index)
                            } else if (model.date !== "") {
                                // date is ISO text for day rows.
                                noteController.jumpToDate(model.date)
                            }
                        }
                    }
                }
            }
        }

        ListView {
            // contentList renders the continuous note stream.
            id: contentList

            // The note stream fills all width not used by the sidebar.
            Layout.fillWidth: true

            // The note stream fills the available application height.
            Layout.fillHeight: true

            // clip keeps paper delegates inside the content viewport.
            clip: true

            // noteModel is provided by main.cpp as a context property.
            model: noteModel

            // spacing separates daily note pages.
            spacing: 20

            // topMargin keeps the first paper page close to the content viewport.
            topMargin: 10

            // bottomMargin keeps the last paper page close to the content viewport.
            bottomMargin: 10

            // DragAndOvershootBounds gives a natural scroll feel across platforms.
            boundsBehavior: Flickable.DragAndOvershootBounds

            delegate: NoteItemDelegate {
                // Delegate width tracks the current content viewport width.
                width: contentList.width
            }

            ScrollBar.vertical: ScrollBar {
                // Always showing the scrollbar makes the continuous document affordance obvious.
                policy: ScrollBar.AlwaysOn
            }

            onContentYChanged: {
                // Reaching the top extends the model with earlier calendar days.
                if (atYBeginning) {
                    noteController.loadPreviousDays(3)
                }

                // Reaching the bottom extends the model with later calendar days.
                if (atYEnd) {
                    noteController.loadNextDays(3)
                }
            }

            function scrollToDateIndex(row) {
                // row is a NoteModel index supplied by NoteController.
                if (row >= 0 && row < count) {
                    // Position the requested note at the top of the viewport.
                    positionViewAtIndex(row, ListView.Beginning)

                    // currentIndex tracks the focused/selected note row.
                    currentIndex = row
                }
            }
        }
    }

    Connections {
        // noteController emits scroll requests after startup or navigation clicks.
        target: noteController

        function onScrollToIndexRequested(index) {
            // index is the NoteModel row that should become visible.
            contentList.scrollToDateIndex(index)
        }
    }

    onClosing: {
        // Flush pending text edits before the window closes normally.
        noteController.flush()
    }
}
