/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtCore 6.5
import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 6.3 as QtDialogs
import org.kde.baloo.experimental 0.1 as Baloo
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcm 1.3 as KCM

KCM.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 42
    implicitHeight: Kirigami.Units.gridUnit * 25

    Connections {
        target: kcm
        function onIndexingSettingsChanged() {
            needToRebootAfterChangingIndexingSettingsMessage.visible = true;
        }
    }
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
            onTriggered: monitor.toggleSuspendState()
        }
    ]

    header: Item {
        implicitHeight: headerColumn.implicitHeight + headerColumn.anchors.topMargin + Kirigami.Units.smallSpacing

        ColumnLayout {
            id: headerColumn

            anchors {
                top: parent.top
                topMargin: Kirigami.Units.largeSpacing
                left: parent.left
                leftMargin: Kirigami.Units.largeSpacing
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing
            }

            spacing: Kirigami.Units.smallSpacing

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                visible: !fileSearchEnabled.checked && kcm.needsSave
                type: Kirigami.MessageType.Warning
                showCloseButton: true
                text: i18n("This will disable file searching in KRunner and launcher menus, and remove extended metadata display from all KDE applications.");
            }

            Kirigami.InlineMessage {
                id: indexingDisabledWarning
                Layout.fillWidth: true
                visible: !kcm.balooSettings.indexingEnabled && !kcm.needsSave && kcm.rawIndexFileSize() > 0
                type: Kirigami.MessageType.Warning
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

            // TODO: remove this message (and the associated Connections object here
            // plus the signal and function on the cpp side) once
            // https://bugs.kde.org/show_bug.cgi?id=462009 is fixed because indexing
            // workers either respect changed settings immediately, or else get killed
            // and re-launched in the background
            Kirigami.InlineMessage {
                id: needToRebootAfterChangingIndexingSettingsMessage
                Layout.fillWidth: true
                visible: false
                type: Kirigami.MessageType.Information
                showCloseButton: true
                text: i18n("The system must be restarted before these changes will take effect.");
                actions: [
                    Kirigami.Action {
                        icon.name: "system-reboot"
                        text: i18nc("@action:button", "Restart")
                        onTriggered: kcm.requestReboot();
                    }
                ]
            }

            QQC2.Label {
                text: i18n("File Search helps you quickly locate your files. You can choose which folders and what types of file data are indexed.")
                Layout.fillWidth: true
                Layout.maximumWidth: Kirigami.Units.gridUnit * 24
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: Kirigami.Units.largeSpacing
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }


            Kirigami.FormLayout {
                id: indexingForm

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

                // Current status
                QQC2.Label {
                    Kirigami.FormData.label: i18nc("@label indexing status", "Status:")
                    Layout.fillWidth: true
                    leftPadding: fileSearchEnabled.indicator.width
                    visible: fileSearchEnabled.checked
                    text: i18n("%1, %2\% complete", monitor.stateString, monitor.completionPercentage)
                    elide: Text.ElideLeft
                }

                // File being indexed, if indexing
                Kirigami.SelectableLabel {
                    readonly property string fileName: monitor.filePath.split('\\').pop().split('/').pop()

                    Kirigami.FormData.label: i18nc("@label file currently being indexed", "Currently Indexing:")
                    Layout.fillWidth: true
                    visible: fileSearchEnabled.checked && monitor.currentlyIndexing && monitor.completionPercentage !== 100 && fileName.length > 0
                    text: xi18nc("@info Currently Indexing", "<filename>%1</filename>", fileName)
                }
            }

            Item {
                implicitHeight: Kirigami.Units.smallSpacing
            }

            Kirigami.FormLayout {
                twinFormLayouts: indexingForm

                Layout.bottomMargin: Kirigami.Units.largeSpacing * 2

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
        header: Rectangle {
            z: 999 // don't let content overlap it
            implicitWidth: directoryConfigList.width
            implicitHeight: listViewHeaderlayout.implicitHeight + (2 * listViewHeaderlayout.anchors.topMargin)

            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
            // We want a color that's basically halfway between the view background
            // color and the window background color. But Due to the use of color
            // scopes, only one will be available at a time. So to get basically the
            // same thing, we blend the view background color with a smidgen of the
            // text color.
            color: Qt.tint(Kirigami.Theme.backgroundColor,
                           Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.03))

            RowLayout {
                id: listViewHeaderlayout

                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.largeSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                    top: parent.top
                    topMargin: Kirigami.Units.smallSpacing
                }

                spacing: 0

                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 2
                    text: i18nc("@title:table Locations to include or exclude from indexing", "Locations")
                    enabled: fileSearchEnabled.checked
                }

                QQC2.ToolButton {
                    text: i18nc("@action:button", "Start Indexing a Folder…")
                    icon.name: "list-add"
                    visible: text.length > 0

                    onClicked: {
                        fileDialogLoader.included = true
                        fileDialogLoader.active = true
                    }
                }

                QQC2.ToolButton {
                    text: i18nc("@action:button", "Stop Indexing a Folder…")
                    icon.name: "list-remove"

                    onClicked: {
                        fileDialogLoader.included = false
                        fileDialogLoader.active = true
                    }
                }
            }

            Kirigami.Separator {
                width: parent.width
                anchors.top: parent.bottom
            }
        }
    }

    Component {
        id: directoryConfigDelegate
        Kirigami.AbstractListItem {
            id: listItem

            // Store this as a property so we can access it within the combobox,
            // which also has a `model` property
            property var indexingModel: model

            // There's no need for a list item to ever be selected
            down: false
            highlighted: false
            hoverEnabled: false
            // ... and because of that, use alternating backgrounds to visually
            // connect list items' left and right side content elements
            alternatingBackground: true

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

                    icon.name: "edit-delete"

                    onClicked: kcm.filteredModel.removeFolder(index)

                    QQC2.ToolTip {
                        text: i18n("Delete entry")
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
