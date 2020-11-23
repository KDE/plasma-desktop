/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *  Copyright 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.3 as QtControls
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.configuration 2.0

Rectangle {
    id: root
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 30
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 20

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    color: Kirigami.Theme.backgroundColor
    width: PlasmaCore.Units.gridUnit * 40
    height: PlasmaCore.Units.gridUnit * 30

    property bool isContainment: false

    property ConfigModel globalConfigModel:  globalAppletConfigModel

    ConfigModel {
        id: globalAppletConfigModel
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Keyboard Shortcuts")
            icon: "preferences-desktop-keyboard"
            source: "ConfigurationShortcuts.qml"
        }
    }

    PlasmaCore.SortFilterModel {
        id: configDialogFilterModel
        sourceModel: configDialog.configModel
        filterRole: "visible"
        filterCallback: function(source_row, value) { return value; }
    }

    function settingValueChanged() {
        applyButton.enabled = true;
    }

    function open(item) {

        if (item.source) {
            if (item.source === "ConfigurationContainmentAppearance.qml") {
                mainLoader.source = item.source
            } else {
                mainLoader.setSource(Qt.resolvedUrl("ConfigurationAppletPage.qml"), {configItem: item})
            }
        } else if (item.kcm) {
            mainLoader.setSource(Qt.resolvedUrl("ConfigurationKcmPage.qml"), {kcm: item.kcm})
        } else {
            mainLoader.setSource("")
        }
        pageTitle.text = item.name

        applyButton.enabled = false
    }

    Connections {
        target: mainLoader.item

        function onSettingValueChanged() {
            applyButton.enabled = true
        }
    }

    Component.onCompleted: {
        // if we are a containment then the first item will be ConfigurationContainmentAppearance
        // if the applet does not have own configs then the first item will be Shortcuts
        if (isContainment || !configDialog.configModel || configDialog.configModel.count === 0) {
            open(root.globalConfigModel.get(0))
        } else {
            open(configDialog.configModel.get(0))
        }
    }

    Rectangle {
        id: sidebar
        anchors.left: root.left
        width: categoriesScroll.width
        height: root.height
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        color: Kirigami.Theme.backgroundColor
    }

    Kirigami.Separator {
        anchors.left: sidebar.right
        height: root.height
        z: 100
    }

    Kirigami.Separator {
        id: topSeparator
        anchors.top: root.top
        width: root.width
    }

    MessageDialog {
        id: messageDialog
        icon: StandardIcon.Warning
        property var item
        title: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply Settings")
        text: i18nd("plasma_shell_org.kde.plasma.desktop", "The settings of the current module have changed. Do you want to apply the changes or discard them?")
        standardButtons: StandardButton.Apply | StandardButton.Discard | StandardButton.Cancel
        onApply: {
            applyAction.trigger()
            root.open(item)
        }
        onDiscard: {
            root.open(item)
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        QtControls.ScrollView {
            id: categoriesScroll
            Layout.fillHeight: true
            visible: true
            clip: true
            Layout.preferredWidth: PlasmaCore.Units.gridUnit * 7
            contentWidth: -1

            Keys.onUpPressed: {
                var buttons = categories.children

                var foundPrevious = false
                for (var i = buttons.length - 1; i >= 0; --i) {
                    var button = buttons[i];
                    if (!button.hasOwnProperty("current")) {
                        // not a ConfigCategoryDelegate
                        continue;
                    }

                    if (foundPrevious) {
                        categories.openCategory(button.item)
                        return
                    } else if (button.current) {
                        foundPrevious = true
                    }
                }
            }

            Keys.onDownPressed: {
                var buttons = categories.children

                var foundNext = false
                for (var i = 0, length = buttons.length; i < length; ++i) {
                    var button = buttons[i];
                    if (!button.hasOwnProperty("current")) {
                        continue;
                    }

                    if (foundNext) {
                        categories.openCategory(button.item)
                        return
                    } else if (button.current) {
                        foundNext = true
                    }
                }
            }

            ColumnLayout {
                id: categories
                spacing: 0
                width: categoriesScroll.availableWidth

                property Item currentItem: children[1]

                function openCategory(item) {
                    if (applyButton.enabled) {
                        messageDialog.item = item;
                        messageDialog.open();
                        return;
                    }
                    open(item)
                }

                Component {
                    id: categoryDelegate
                    ConfigCategoryDelegate {
                        onActivated: categories.openCategory(model)
                        current: {
                            if (model.kcm && mainLoader.item.kcm) {
                                return model.kcm == mainLoader.item.kcm
                            }

                            if (mainLoader.item.configItem) {
                                return model.source == mainLoader.item.configItem.source
                            }

                            return mainLoader.source == Qt.resolvedUrl(model.source)
                        }
                        item: model
                    }
                }

                Repeater {
                    model: root.isContainment ? globalConfigModel : undefined
                    delegate: categoryDelegate
                }
                Repeater {
                    model: configDialogFilterModel
                    delegate: categoryDelegate
                }
                Repeater {
                    model: !root.isContainment ? globalConfigModel : undefined
                    delegate: categoryDelegate
                }
                Repeater {
                    model: ConfigModel {
                        ConfigCategory{
                            name: i18nd("plasma_shell_org.kde.plasma.desktop", "About")
                            icon: "help-about"
                            source: "AboutPlugin.qml"
                        }
                    }
                    delegate: categoryDelegate
                }
            }
        }

        // Configuration area and buttons.
        ColumnLayout {
            id: configColumn
            Layout.topMargin: topSeparator.height
            Layout.bottomMargin: PlasmaCore.Units.smallSpacing * 2

            Kirigami.Heading {
                id: pageTitle
                Layout.fillWidth: true
                topPadding: Kirigami.Units.smallSpacing
                leftPadding: Kirigami.Units.largeSpacing
                level: 1
            }

            Loader {
                id: mainLoader
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            QtControls.Action {
                id: acceptAction
                onTriggered: {
                    applyAction.trigger();
                    configDialog.close();
                }
                shortcut: "Return"
            }

            QtControls.Action {
                id: applyAction
                onTriggered: {
                    mainLoader.item.saveConfig()

                    applyButton.enabled = false;
                }
            }

            QtControls.Action {
                id: cancelAction
                onTriggered: configDialog.close();
                shortcut: "Escape"
            }

            RowLayout {
                id: buttonsRow
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: PlasmaCore.Units.smallSpacing * 2
                Layout.leftMargin: PlasmaCore.Units.smallSpacing * 2

                QtControls.Button {
                    icon.name: "dialog-ok"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "OK")
                    onClicked: acceptAction.trigger()
                }
                QtControls.Button {
                    id: applyButton
                    enabled: false
                    icon.name: "dialog-ok-apply"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                    visible: mainLoader.item && (!mainLoader.item.kcm || mainLoader.item.kcm.buttons & 4) // 4 = Apply button
                    onClicked: applyAction.trigger()
                }
                QtControls.Button {
                    icon.name: "dialog-cancel"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                    onClicked: cancelAction.trigger()
                }
            }
        }
    }
}
