# Require a CMake version new enough for modern Qt 6 helper functions.
cmake_minimum_required(VERSION 3.21)

# Define the ABCNote project and select C++ as the only compiled language.
project(ABCNote VERSION 1.0.1 LANGUAGES CXX)

# CMAKE_CXX_STANDARD selects the language level used by all targets.
set(CMAKE_CXX_STANDARD 17)

# CMAKE_CXX_STANDARD_REQUIRED prevents CMake from silently using an older standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE_AUTOMOC lets Qt generate meta-object code for QObject classes automatically.
set(CMAKE_AUTOMOC ON)

# Locate the Qt modules used by the app and tests.
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Network Test)

# Apply Qt's standard project settings for Qt 6.5 and newer.
qt_standard_project_setup(REQUIRES 6.5)

# ABCNOTE_SOURCES lists implementation files shared by the app and test executable.
set(ABCNOTE_SOURCES
    src/AppSettings.cpp
    src/Localization.cpp
    src/NoteStorage.cpp
    src/SummaryService.cpp
    src/NoteModel.cpp
    src/NavigationModel.cpp
    src/NoteController.cpp
)

# ABCNOTE_HEADERS lists public project headers for IDE visibility and AUTOMOC scanning.
set(ABCNOTE_HEADERS
    src/AppSettings.h
    src/BuildInfo.h
    src/Localization.h
    src/Note.h
    src/NoteStorage.h
    src/SummaryService.h
    src/NoteModel.h
    src/NavigationModel.h
    src/NoteController.h
)

# QT_RESOURCE_ALIAS makes Main.qml load from the QML module root.
set_source_files_properties(Layout/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)

# QT_RESOURCE_ALIAS makes NoteItemDelegate.qml importable beside Main.qml.
set_source_files_properties(Layout/NoteItemDelegate.qml PROPERTIES QT_RESOURCE_ALIAS NoteItemDelegate.qml)

# ABCNote is the GUI application target.
qt_add_executable(ABCNote
    WIN32
    src/main.cpp
    src/app.rc
    assets/app.qrc
    ${ABCNOTE_SOURCES}
    ${ABCNOTE_HEADERS}
)

# The ABCNote QML module embeds the QML files into the executable resources.
qt_add_qml_module(ABCNote
    URI ABCNote
    VERSION 1.0
    QML_FILES
        Layout/Main.qml
        Layout/NoteItemDelegate.qml
)

# Link the application against the Qt modules it uses at runtime.
target_link_libraries(ABCNote
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Network
        Qt6::Quick
        Qt6::QuickControls2
)

if(WIN32)
    target_link_libraries(ABCNote PRIVATE Advapi32)
endif()

# Add src to the include path so source files can include project headers directly.
target_include_directories(ABCNote PRIVATE src)

# MSVC needs /utf-8 so escaped and any future literal strings are interpreted consistently.
if(MSVC)
    target_compile_options(ABCNote PRIVATE /utf-8)
endif()

# Enable CTest integration for the test target.
enable_testing()

# ABCNoteTests is the Qt Test executable for non-UI behavior.
qt_add_executable(ABCNoteTests
    tests/tst_abcnote.cpp
    ${ABCNOTE_SOURCES}
    ${ABCNOTE_HEADERS}
)

# Add src to the test include path so tests can include project headers directly.
target_include_directories(ABCNoteTests PRIVATE src)

# MSVC needs /utf-8 for test string literals as well.
if(MSVC)
    target_compile_options(ABCNoteTests PRIVATE /utf-8)
endif()

# Link tests against Qt Core and Qt Test because they do not load QML.
target_link_libraries(ABCNoteTests
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Test
)

if(WIN32)
    target_link_libraries(ABCNoteTests PRIVATE Advapi32)
endif()

# Register the Qt Test executable with CTest.
add_test(NAME ABCNoteTests COMMAND ABCNoteTests)
