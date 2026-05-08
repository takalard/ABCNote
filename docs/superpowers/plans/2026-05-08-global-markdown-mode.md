# Global Markdown Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add one persisted toolbar Edit/Preview Markdown mode that controls all daily notes.

**Architecture:** Store the UI preference in `AppSettings`, because it already owns `ABCNote.ini` user preferences and is exposed to QML. The toolbar writes the setting, and every `NoteItemDelegate` reads the same property to decide whether to show the editor or Markdown preview.

**Tech Stack:** Qt 6, C++, QSettings, QML, QtTest.

---

### Task 1: Add Persisted Markdown Mode Setting

**Files:**
- Modify: `tests/tst_abcnote.cpp`
- Modify: `src/AppSettings.h`
- Modify: `src/AppSettings.cpp`

- [ ] **Step 1: Write the failing test**

Add the test declaration in `tests/tst_abcnote.cpp` inside `private slots:` after `appSettingsPersistsAiSummarySettings();`:

```cpp
    // Verifies Markdown preview mode is a global UI preference persisted in settings.
    void appSettingsPersistsMarkdownPreviewMode();
```

Add this test body before `ABCNoteTests::appSettingsClearsAiApiKey()`:

```cpp
void ABCNoteTests::appSettingsPersistsMarkdownPreviewMode()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    QVERIFY(QDir().mkpath(appDir));

    AppSettings first(settingsPath, appDir);
    QVERIFY(!first.markdownPreviewMode());
    QSignalSpy changedSpy(&first, &AppSettings::markdownPreviewModeChanged);

    QVERIFY(first.setMarkdownPreviewMode(true));
    QVERIFY(first.markdownPreviewMode());
    QCOMPARE(changedSpy.count(), 1);

    AppSettings second(settingsPath, appDir);
    QVERIFY(second.markdownPreviewMode());

    QVERIFY(second.setMarkdownPreviewMode(false));
    QVERIFY(!second.markdownPreviewMode());

    AppSettings third(settingsPath, appDir);
    QVERIFY(!third.markdownPreviewMode());
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: build fails because `AppSettings` does not have `markdownPreviewMode`, `setMarkdownPreviewMode`, or `markdownPreviewModeChanged`.

- [ ] **Step 3: Add the AppSettings API**

In `src/AppSettings.h`, add this property after `statusMessage`:

```cpp
    // Whether all note bodies render as Markdown preview instead of editable text.
    Q_PROPERTY(bool markdownPreviewMode READ markdownPreviewMode NOTIFY markdownPreviewModeChanged)
```

Add this public getter after `statusMessage() const;`:

```cpp
    // Returns whether daily notes are shown in Markdown preview mode.
    bool markdownPreviewMode() const;
```

Add this invokable after `migrateAndSwitchDataRootUrl(...)`:

```cpp
    // Persists and applies the global Markdown preview mode.
    Q_INVOKABLE bool setMarkdownPreviewMode(bool enabled);
```

Add this signal after `statusMessageChanged();`:

```cpp
    // Emitted when markdownPreviewMode changes.
    void markdownPreviewModeChanged();
```

Add this private loader after `loadDataRoot() const;`:

```cpp
    // Loads the global Markdown preview preference from QSettings.
    bool loadMarkdownPreviewMode() const;
```

Add this member after `m_dataRoot`:

```cpp
    // Global UI mode for all note body delegates.
    bool m_markdownPreviewMode = false;
```

- [ ] **Step 4: Add the AppSettings implementation**

In `src/AppSettings.cpp`, initialize the member in the constructor initializer list:

```cpp
    , m_markdownPreviewMode(loadMarkdownPreviewMode())
```

Add this getter after `statusMessage()`:

```cpp
bool AppSettings::markdownPreviewMode() const
{
    return m_markdownPreviewMode;
}
```

Add this method after `migrateAndSwitchDataRootUrl(...)`:

```cpp
bool AppSettings::setMarkdownPreviewMode(bool enabled)
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("ui/markdownPreviewMode"), enabled);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    if (m_markdownPreviewMode != enabled) {
        m_markdownPreviewMode = enabled;
        emit markdownPreviewModeChanged();
    }
    return true;
}
```

Add this loader after `loadDataRoot()`:

```cpp
bool AppSettings::loadMarkdownPreviewMode() const
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    return settings.value(QStringLiteral("ui/markdownPreviewMode"), false).toBool();
}
```

- [ ] **Step 5: Run test to verify it passes**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: all tests pass.

- [ ] **Step 6: Commit**

```powershell
git add tests/tst_abcnote.cpp src/AppSettings.h src/AppSettings.cpp
git commit -m "Add persisted markdown preview mode setting"
```

### Task 2: Move Edit/Preview Control To Toolbar

**Files:**
- Modify: `Layout/Main.qml`
- Modify: `Layout/NoteItemDelegate.qml`

- [ ] **Step 1: Update the toolbar**

In `Layout/Main.qml`, add this segmented control in the toolbar `RowLayout` immediately before `ToolButton { id: settingsButton ... }`:

```qml
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
```

- [ ] **Step 2: Remove the per-note mode state and switch**

In `Layout/NoteItemDelegate.qml`, remove:

```qml
            property bool previewMode: false
```

Remove the `RowLayout` block whose comment is:

```qml
                // Mode switch keeps editing and Markdown preview on the same daily page.
```

- [ ] **Step 3: Bind note body visibility to AppSettings**

In `Layout/NoteItemDelegate.qml`, change the editor visibility to:

```qml
                visible: !appSettings.markdownPreviewMode
```

Change the preview panel visibility to:

```qml
                visible: appSettings.markdownPreviewMode
```

- [ ] **Step 4: Run tests**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: all tests pass.

- [ ] **Step 5: Commit**

```powershell
git add Layout/Main.qml Layout/NoteItemDelegate.qml
git commit -m "Move markdown mode switch to toolbar"
```

### Task 3: Manual UI Check

**Files:**
- Read: `README.md`
- Run: local application through the existing build output or documented Qt workflow.

- [ ] **Step 1: Launch the app**

Run the documented build or launch command available in the repository. If the build scripts produced an executable, start ABCNote from that output.

- [ ] **Step 2: Verify toolbar mode behavior**

Expected:

- The Edit/Preview switch appears next to the Settings gear.
- Each daily note no longer has its own Edit/Preview switch.
- Clicking Preview changes every visible daily note body to Markdown preview.
- Clicking Edit changes every visible daily note body back to editable text.

- [ ] **Step 3: Verify persistence**

Expected:

- Choose Preview.
- Close and reopen ABCNote.
- The toolbar still shows Preview selected and all daily notes open in preview mode.
- Choose Edit, close and reopen again.
- The toolbar opens in edit mode.
