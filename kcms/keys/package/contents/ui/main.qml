/*
    SPDX-FileCopyrightText: 2020 David Redondo <david@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import QtQml 2.14
import QtQml.Models 2.3

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcm 1.4 as KCM
import org.kde.private.kcms.keys 2.0 as Private

KCM.AbstractKCM {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 33
    property alias exportActive: exportInfo.visible
    readonly property bool errorOccured: kcm.lastError != ""
    ColumnLayout {
        anchors.fill: parent
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: errorOccured
            text: kcm.lastError
            type: Kirigami.MessageType.Error
        }
        Kirigami.InlineMessage {
            id: exportWarning
            Layout.fillWidth: true
            text: i18n("Cannot export scheme while there are unsaved changes")
            type: Kirigami.MessageType.Warning
            showCloseButton: true
            Binding on visible {
                when: exportWarning.visible
                value: kcm.needsSave
                restoreMode: Binding.RestoreNone
            }
        }
        Kirigami.InlineMessage {
            id: exportInfo
            Layout.fillWidth: true
            text: i18n("Select the components below that should be included in the exported scheme")
            type: Kirigami.MessageType.Information
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    iconName: "document-save"
                    text: i18n("Save scheme")
                    onTriggered: {
                        fileDialogLoader.save = true
                        fileDialogLoader.active = true
                        exportActive = false
                    }
                }
            ]
        }
        Kirigami.SearchField  {
            id: search
            enabled: !errorOccured && !exportActive
            Layout.fillWidth: true
            Binding {
                target: kcm.filteredModel
                property: "filter"
                value: search.text
                restoreMode: Binding.RestoreBinding
            }
        }
        GridLayout  {
            enabled: !errorOccured
            columns: 2
            QQC2.ScrollView {
                Component.onCompleted:  background.visible = true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 15
                Layout.fillHeight:true
                ListView {
                    id: components
                    clip: true
                    model: kcm.filteredModel
                    activeFocusOnTab: true
                    add: Transition {
                        id: transition
                        PropertyAction {
                            target: components
                            property: "currentIndex"
                            value: transition.ViewTransition.index
                        }
                    }

                    // Not using CheckableListItem despite having a checkbox because
                    // the list item is not always in a checkable state, so the checkbox
                    // does not always need to be visible, but CheckableListItem
                    // makes that assumption.
                    delegate: Kirigami.BasicListItem {
                        id: componentDelegate

                        height: deleteButton.height + (Kirigami.Units.smallSpacing * 2)

                        fadeContent: model.pendingDeletion

                        KeyNavigation.right: shortcutsList

                        icon: model.decoration
                        label: model.display

                        trailing: RowLayout {
                            spacing: Kirigami.Units.smallSpacing
                            QQC2.CheckBox {
                                checked: model.checked
                                visible: exportActive
                                onToggled: model.checked = checked
                            }
                            QQC2.Button {
                                id: deleteButton

                                implicitHeight: Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing
                                implicitWidth: implicitHeight

                                visible: model.section != i18n("Common Actions") // FIXME: don't compare translated strings
                                         && !exportActive
                                         && !model.pendingDeletion
                                         && (componentDelegate.containsMouse || componentDelegate.ListView.isCurrentItem)
                                icon.name: "edit-delete"
                                onClicked: model.pendingDeletion = true
                                QQC2.ToolTip {
                                    text: i18n("Remove all shortcuts for %1", model.display)
                                }
                            }
                            QQC2.Button {
                                implicitHeight: Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.smallSpacing
                                implicitWidth: implicitHeight

                                visible: !exportActive && model.pendingDeletion
                                icon.name: "edit-undo"
                                onClicked: model.pendingDeletion = false
                                QQC2.ToolTip {
                                    text: i18n("Undo deletion")
                                }
                            }
                            Rectangle {
                                id: defaultIndicator
                                radius: width * 0.5
                                implicitWidth: Kirigami.Units.largeSpacing
                                implicitHeight: Kirigami.Units.largeSpacing
                                visible: kcm.defaultsIndicatorsVisible
                                opacity: !model.isDefault
                                color: Kirigami.Theme.neutralTextColor
                            }
                        }
                    }
                    section.property: "section"
                    section.delegate: Kirigami.ListSectionHeader {
                        label: section
                        QQC2.CheckBox {
                            id: sectionCheckbox
                            Layout.alignment: Qt.AlignRight
                            // width of indicator + layout spacing
                            Layout.rightMargin: kcm.defaultsIndicatorsVisible ? Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing : 0
                            visible: exportActive
                            onToggled: {
                                const checked = sectionCheckbox.checked
                                const startIndex = kcm.shortcutsModel.index(0, 0)
                                const indices = kcm.shortcutsModel.match(startIndex, Private.BaseModel.SectionRole, section, -1, Qt.MatchExactly)
                                for (const index of indices) {
                                    kcm.shortcutsModel.setData(index, checked, Private.BaseModel.CheckedRole)
                                }
                            }
                            Connections {
                                enabled: exportActive
                                target: kcm.shortcutsModel
                                function onDataChanged (topLeft, bottomRight, roles) {
                                    const startIndex = kcm.shortcutsModel.index(0, 0)
                                    const indices = kcm.shortcutsModel.match(startIndex, Private.BaseModel.SectionRole, section, -1, Qt.MatchExactly)
                                    sectionCheckbox.checked = indices.reduce((acc, index) => acc && kcm.shortcutsModel.data(index, Private.BaseModel.CheckedRole), true)
                                }
                            }
                        }
                    }
                    onCurrentItemChanged: dm.rootIndex = kcm.filteredModel.index(currentIndex, 0)
                    onCurrentIndexChanged:{ shortcutsList.selectedIndex = -1;
                    }
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                QQC2.ScrollView  {
                    enabled: !exportActive
                    id: shortcutsScroll
                    anchors.fill:parent
                    clip: true
                    Component.onCompleted: background.visible = true
                    ListView {
                        clip:true
                        id: shortcutsList
                        property int selectedIndex: -1
                        model: DelegateModel {
                            id: dm
                            model: rootIndex.valid ?  kcm.filteredModel : undefined
                            delegate: ShortcutActionDelegate {}
                            KeyNavigation.left: components
                        }
                    }
                }
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: components.currentIndex == -1
                    text: i18n("Select an item from the list to view its shortcuts here")
                }
            }
            QQC2.Button {
                enabled: !exportActive
                Layout.alignment: Qt.AlignRight
                icon.name: "list-add"
                text: i18n("Add Application…")
                onClicked: {
                    kcm.addApplication(this)
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignRight
                QQC2.Button {
                    enabled: !exportActive
                    icon.name: "document-import"
                    text: i18n("Import Scheme…")
                    onClicked: importSheet.open()
                }
                QQC2.Button {
                    icon.name: exportActive ? "dialog-cancel" : "document-export"
                    text: exportActive ? i18n("Cancel Export") : i18n("Export Scheme…")
                    onClicked: {
                        if (exportActive) {
                            exportActive = false
                        } else if (kcm.needsSave) {
                            exportWarning.visible = true
                        } else {
                            search.text = ""
                            exportActive = true
                        }
                    }
                }
            }
        }
    }
    Loader {
        id: fileDialogLoader
        active: false
        property bool save
        sourceComponent: FileDialog {
            id: fileDialog
            title: save ? i18n("Export Shortcut Scheme") : i18n("Import Shortcut Scheme")
            folder: shortcuts.home
            nameFilters: [ i18nc("Template for file dialog","Shortcut Scheme (*.kksrc)") ]
            defaultSuffix: ".kksrc"
            selectExisting: !fileDialogLoader.save
            Component.onCompleted: open()
            onAccepted: {
                if (save) {
                    kcm.writeScheme(fileUrls[0])
                } else {
                    var schemes = schemeBox.model
                    schemes.splice(schemes.length - 1, 0, {name: kcm.urlFilename(fileUrls[0]), url: fileUrls[0]})
                    schemeBox.model = schemes
                    schemeBox.currentIndex = schemes.length - 2
                }
                fileDialogLoader.active = false
            }
            onRejected: fileDialogLoader.active = false
        }
    }
    Kirigami.OverlaySheet {
        id: importSheet

        title: i18n("Import Shortcut Scheme")

        ColumnLayout {
            anchors.centerIn: parent
            QQC2.Label {
                text: i18n("Select the scheme to import:")
            }
            RowLayout {
                QQC2.ComboBox {
                    id: schemeBox
                    readonly property bool customSchemeSelected: currentIndex == count - 1
                    property string url: ""
                    currentIndex: count - 1
                    textRole: "name"
                    onActivated: url = model[index]["url"]
                    Component.onCompleted: {
                        var defaultSchemes = kcm.defaultSchemes()
                        defaultSchemes.push({name: i18n("Custom Scheme"), url: "unused"})
                        model = defaultSchemes
                    }
                }
                QQC2.Button {
                    text: schemeBox.customSchemeSelected ? i18n("Select File…") : i18n("Import")
                    onClicked: {
                        if (schemeBox.customSchemeSelected) {
                            fileDialogLoader.save = false;
                            fileDialogLoader.active = true;
                        } else {
                            kcm.loadScheme(schemeBox.model[schemeBox.currentIndex]["url"])
                            importSheet.close()
                        }
                    }
                }
            }
        }
    }
}

