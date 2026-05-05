import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // Delegate width is assigned by Main.qml to match the content ListView.
    width: parent.width

    // height follows the paper canvas so each date page occupies stable space in the ListView.
    height: pageCanvas.height + 10

    Rectangle {
        // pageCanvas is the white paper-like surface for one daily note.
        id: pageCanvas

        // Width is responsive but capped for comfortable reading.
        width: Math.min(parent.width * 0.9, 860)

        // Height expands with editor content while preserving a useful minimum.
        height: Math.max(420, contentColumn.implicitHeight + 80)

        // Center the page in the content stream.
        anchors.centerIn: parent

        // White background gives a document/page feel.
        color: "white"

        // radius is small to keep the page professional rather than playful.
        radius: 5

        // border.color separates paper from the gray app background.
        border.color: "#dddddd"

        // layer.enabled allows Qt Quick to cache this page surface efficiently.
        layer.enabled: true

        ColumnLayout {
            // contentColumn stacks title, editor, divider, and summary.
            id: contentColumn

            // Fill the paper page while respecting margins.
            anchors.fill: parent

            // margins create the writing area inside the paper.
            anchors.margins: 40

            // spacing separates daily note sections.
            spacing: 16

            Text {
                // dateString is provided by NoteModel as the localized note title.
                text: dateString

                // Title typography distinguishes each daily note.
                font.pixelSize: 24

                // Bold date title improves scanning while scrolling.
                font.bold: true

                // Dark color gives strong contrast on the white page.
                color: "#1a1a1a"

                // Fill width so long localized dates wrap if needed.
                Layout.fillWidth: true
            }

            TextArea {
                // bodyEditor is the editable note body for this date.
                id: bodyEditor

                // contentBody is the NoteModel role holding the saved body text.
                text: contentBody

                // Placeholder follows the selected UI language.
                placeholderText: i18n.t("notes.bodyPlaceholder")

                // Wrap keeps writing inside the page width.
                wrapMode: TextArea.Wrap

                // Body text size is compact and desktop-note friendly.
                font.pixelSize: 14

                // Body text color is dark but softer than pure black.
                color: "#222222"

                // selectedTextColor ensures selected text remains readable.
                selectedTextColor: "white"

                // selectionColor uses the app accent blue.
                selectionColor: "#2b6fd6"

                // Fill the paper writing width.
                Layout.fillWidth: true

                // preferredHeight grows with content but starts with a useful empty-note size.
                Layout.preferredHeight: Math.max(180, contentHeight + 24)

                // Transparent background makes typing feel like writing on paper.
                background: Rectangle { color: "transparent" }

                onTextChanged: {
                    // activeFocus prevents initial model binding assignment from being treated as a user edit.
                    if (activeFocus) {
                        noteController.updateNoteBody(noteId, text)
                    }
                }
            }

            Rectangle {
                // Divider separates the editable body from the generated summary.
                height: 1

                // Divider spans the writing width.
                Layout.fillWidth: true

                // Light gray keeps the divider subtle.
                color: "#eeeeee"
            }

            Rectangle {
                // Summary panel contains the generated AI/local summary.
                Layout.fillWidth: true

                // preferredHeight follows the summary content and refresh button row.
                Layout.preferredHeight: summaryColumn.implicitHeight + 22

                // Pale blue background distinguishes generated text from user text.
                color: "#f4f8ff"

                // Rounded corners are modest and contained within the note page.
                radius: 8

                // Border reinforces the summary panel boundary.
                border.color: "#d7e6ff"

                ColumnLayout {
                    // summaryColumn stacks the label row and summary text.
                    id: summaryColumn

                    // Horizontal anchors fill the summary panel.
                    anchors.left: parent.left
                    anchors.right: parent.right

                    // Vertical centering keeps short summaries balanced.
                    anchors.verticalCenter: parent.verticalCenter

                    // margins keep text away from the panel edge.
                    anchors.margins: 12

                    // spacing separates the label row from summary text.
                    spacing: 8

                    RowLayout {
                        // Label row fills the panel width so the button can align right.
                        Layout.fillWidth: true

                        Text {
                            // Label follows the selected UI language.
                            text: i18n.t("notes.aiSummary")

                            // Bold label makes the generated section easy to identify.
                            font.bold: true

                            // Accent color links the label to the summary panel style.
                            color: "#2b579a"
                        }

                        Item {
                            // Spacer pushes the refresh button to the right edge.
                            Layout.fillWidth: true
                        }

                        Button {
                            // Refresh label follows the selected UI language.
                            text: i18n.t("notes.refresh")

                            // flat style keeps the button lightweight inside the note page.
                            flat: true

                            // Refresh regenerates the summary for this delegate's note id.
                            onClicked: noteController.refreshSummary(noteId)
                        }
                    }

                    Text {
                        // aiText renders the generated summary content.
                        id: aiText

                        // aiSummary is the NoteModel role for generated summary text.
                        text: aiSummary

                        // Wrap summary text within the panel width.
                        wrapMode: Text.Wrap

                        // Italic style differentiates generated prose from user-entered prose.
                        font.italic: true

                        // Muted color keeps summary secondary.
                        color: "#666666"

                        // Fill width so wrapping uses all available panel space.
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
