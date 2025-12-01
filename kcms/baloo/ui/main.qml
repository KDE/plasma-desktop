/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as QtDialogs
import org.kde.baloo.experimental as Baloo
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 42
    implicitHeight: Kirigami.Units.gridUnit * 25

    Baloo.Monitor {
        id: monitor

        readonly property bool currentlyIndexing: switch(state) {
            case Baloo.Global.FirstRun:
            case Baloo.Global.NewFiles:
            case Baloo.Global.ModifiedFiles:
            case Baloo.Global.XAttrFiles:
            case Baloo.Global.ContentIndexing:
            case Baloo.Global.UnindexedFileCheck:
            case Baloo.Global.StaleIndexEntriesClean:
                return true;
            default:
                return false;
        }

        readonly property int completionPercentage: Math.floor(filesIndexed / totalFiles * 100)
    }

    actions: [
        Kirigami.Action {
            icon.name: monitor.state !== Baloo.Global.Suspended ? "media-playback-pause" : "media-playback-start"
            text: monitor.state !== Baloo.Global.Suspended ? i18n("Pause Indexer") : i18n("Resume Indexer")
            visible: kcm.balooSettings.indexingEnabled
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: monitor.toggleSuspendState()
        }
    ]

    headerPaddingEnabled: false // Let the InlineMessages touch the edges
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !fileSearchEnabled.checked && kcm.needsSave
            type: Kirigami.MessageType.Warning
            position: Kirigami.InlineMessage.Position.Header
            showCloseButton: true
            text: i18n("This will disable file searching in KRunner and launcher menus, and remove extended metadata display from all KDE applications.");
        }

        Kirigami.InlineMessage {
            id: indexingDisabledWarning
            Layout.fillWidth: true
            visible: !kcm.balooSettings.indexingEnabled && !kcm.needsSave && kcm.rawIndexFileSize() > 0
            type: Kirigami.MessageType.Warning
            position: Kirigami.InlineMessage.Position.Header
            showCloseButton: true
            text: i18n("Do you want to delete the saved index data? %1 of space will be freed, but if indexing is re-enabled later, the entire index will have to be re-created from scratch. This may take some time, depending on how many files you have.", kcm.prettyIndexFileSize());
            actions: Kirigami.Action {
                text: i18n("Delete Index Data")
                icon.name: "edit-delete"
                onTriggered: {
                    kcm.deleteIndex();
                    indexingDisabledWarning.visible = false;
                }
            }
        }

        // By disabling header margins to make the header banners look correct,
        // we have to re-add some margins for the content area so it doesn't
        // touch the window edges and look ugly. To make it apply to everything,
        // we wrap it all in another ColumnLayout with its own margins.
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.margins: Kirigami.Units.gridUnit

            QQC2.Label {
                text: i18n("File Search helps you quickly locate your files. You can choose which folders and what types of file data are indexed.")
                textFormat: Text.PlainText
                Layout.fillWidth: true
                Layout.maximumWidth: Kirigami.Units.gridUnit * 24
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Kirigami.FormLayout {

                QQC2.CheckBox {
                    id: fileSearchEnabled
                    Kirigami.FormData.label: i18nc("@title:group", "File indexing:")
                    text: i18nc("@action:check", "Enabled")
                    checked: kcm.balooSettings.indexingEnabled
                    onCheckStateChanged: {
                        kcm.balooSettings.indexingEnabled = checked
                    }

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "indexingEnabled"
                    }
                }

                QQC2.ButtonGroup {
                    id: indexingStyleGroup
                }
                QQC2.RadioButton {
                    Kirigami.FormData.label: i18nc("@title:group", "Data to index:")

                    text: i18n("File names and contents")
                    checked: !kcm.balooSettings.onlyBasicIndexing
                    onToggled: kcm.balooSettings.onlyBasicIndexing = !checked

                    QQC2.ButtonGroup.group: indexingStyleGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "onlyBasicIndexing"
                        extraEnabledConditions: fileSearchEnabled.checked
                    }
                }
                QQC2.RadioButton {
                    text: i18n("File names only")
                    checked: kcm.balooSettings.onlyBasicIndexing
                    onToggled: kcm.balooSettings.onlyBasicIndexing = checked

                    QQC2.ButtonGroup.group: indexingStyleGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "onlyBasicIndexing"
                        extraEnabledConditions: fileSearchEnabled.checked
                    }
                }

                Item {
                    implicitHeight: Kirigami.Units.smallSpacing
                }

                QQC2.CheckBox {
                    id: indexHiddenFolders
                    text: i18n("Hidden files and folders")
                    checked: kcm.balooSettings.indexHiddenFolders
                    onCheckStateChanged: kcm.balooSettings.indexHiddenFolders = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "indexHiddenFolders"
                        extraEnabledConditions: fileSearchEnabled.checked
                    }
                }
            }
        }
    }

    view: ListView {
        id: directoryConfigList
        enabled: fileSearchEnabled.checked
        clip: true
        currentIndex: -1

        model: kcm.filteredModel
        delegate: directoryConfigDelegate

        headerPositioning: ListView.OverlayHeader
        header: Kirigami.InlineViewHeader {
            width: directoryConfigList.width
            text: i18nc("@title:table Locations to include or exclude from indexing", "Locations")
            actions: [
                Kirigami.Action {
                    text: i18nc("@action:button", "Start Indexing a Folder…")
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        fileDialogLoader.included = true
                        fileDialogLoader.active = true
                    }
                },
                Kirigami.Action {
                    text: i18nc("@action:button", "Stop Indexing a Folder…")
                    icon.name: "list-remove-symbolic"
                    onTriggered: {
                        fileDialogLoader.included = false
                        fileDialogLoader.active = true
                    }
                }
            ]
        }
    }

    extraFooterTopPadding: true // re-add separator line
    footer: ColumnLayout {
        visible: fileSearchEnabled.checked
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            text: i18nc("State and a percentage of progress", "%1, %2% complete", monitor.stateString, monitor.completionPercentage)
            textFormat: Text.PlainText
        }

        Kirigami.SelectableLabel {
            Layout.fillWidth: true
            visible: monitor.currentlyIndexing && monitor.completionPercentage !== 100 && monitor.filePath.length > 0
            text: xi18nc("@info Currently Indexing", "Currently indexing: <filename>%1</filename>", monitor.filePath)
        }
    }

    Component {
        id: directoryConfigDelegate
        QQC2.ItemDelegate {
            id: listItem

            // Store this as a property so we can access it within the combobox,
            // which also has a `model` property
            property var indexingModel: model

            width: directoryConfigList.width

            // There's no need for a list item to ever be selected
            down: false
            highlighted: false
            hoverEnabled: false
            // ... and because of that, use alternating backgrounds to visually
            // connect list items' left and right side content elements
            Kirigami.Theme.useAlternateBackgroundColor: true

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                // The folder's icon
                Kirigami.Icon {
                    source: indexingModel.decoration

                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredWidth: Layout.preferredHeight
                }

                // The folder's path
                QQC2.Label {
                    text: indexingModel.folder
                    textFormat: Text.PlainText
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }

                // What kind of indexing to do for the folder
                QQC2.ComboBox {
                    id: indexingOptionsCombobox

                    property bool indexingDisabled: !indexingModel.enableIndex
                    property bool fullContentIndexing: indexingModel.enableIndex

                    flat: true

                    model: [
                        i18n("Not indexed"),
                        i18n("Indexed"),
                    ]

                    // Intentionally not a simple ternary to facilitate adding
                    // more conditions in the future
                    currentIndex: {
                        if (indexingDisabled) return 0
                        if (fullContentIndexing) return 1
                    }

                    onActivated: {
                        // New value is "Not indexed"
                        if (indexingOptionsCombobox.currentIndex === 0 && fullContentIndexing) {
                            indexingModel.enableIndex = false
                        // New value is "Full content indexing"
                        } else if (indexingOptionsCombobox.currentIndex === 1 && indexingDisabled) {
                            indexingModel.enableIndex = true
                        }
                    }
                }

                // Delete button to remove this folder entry
                QQC2.ToolButton {
                    enabled: model.deletable

                    icon.name: "edit-delete-remove"

                    onClicked: kcm.filteredModel.removeFolder(index)

                    QQC2.ToolTip {
                        text: i18nc("Remove the list item for this filesystem path", "Remove entry")
                    }
                }
            }
        }
    }

    Loader {
        id: fileDialogLoader

        property bool included: false

        active: false

        sourceComponent: QtDialogs.FolderDialog {
            title: fileDialogLoader.included ? i18n("Select a folder to include") : i18n("Select a folder to exclude")
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]

            onAccepted: {
                kcm.filteredModel.addFolder(selectedFolder, fileDialogLoader.included)
                fileDialogLoader.active = false
            }

            onRejected: {
                fileDialogLoader.active = false
            }

            Component.onCompleted: open()
        }
    }
}
