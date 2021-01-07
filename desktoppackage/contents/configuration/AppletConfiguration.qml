/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *  Copyright 2020 Nicolas Fella <nicolas.fella@gmx.de>
 *  Copyright 2020 Carl Schwan <carlschwan@kde.org>
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

import QtQuick 2.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.3 as QtControls
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.configuration 2.0

Rectangle {
    id: root

    implicitWidth: PlasmaCore.Units.gridUnit * 40
    implicitHeight: PlasmaCore.Units.gridUnit * 31

    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 40
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 31

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    color: Kirigami.Theme.backgroundColor

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

    function pushReplace(item, config) {
        let page;
        if (app.pageStack.depth === 1) {
            page = app.pageStack.push(item, config);
        } else {
            page = app.pageStack.replace(item, config);
        }
        app.currentConfigPage = page;
    }
    Component {
        id: configurationKcmPageComponent
        ConfigurationKcmPage {
        }
    }

    function open(item) {
        app.isAboutPage = false;
        if (item.source) {
            app.isAboutPage = item.source === "AboutPlugin.qml";
            if (item.source === "ConfigurationContainmentAppearance.qml") {
                pushReplace(Qt.resolvedUrl(item.source), {title: item.name});
            } else {
                pushReplace(Qt.resolvedUrl("ConfigurationAppletPage.qml"), {configItem: item, title: item.name});
            }
        } else if (item.kcm) {
            pushReplace(configurationKcmPageComponent, {kcm: item.kcm, internalPage: item.kcm.mainUi});
        } else {
            app.pageStack.pop();
        }

        applyButton.enabled = false
    }

    Connections {
        target: app.currentConfigPage

        function onSettingValueChanged() {
            applyButton.enabled = true;
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

    function applicationWindow() {
        return app;
    }

    Kirigami.ApplicationItem {
        id: app
        anchors.fill: parent

        pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Breadcrumb
        wideScreen: true

        property var currentConfigPage: null
        property bool isAboutPage: false

        // HACK: setting columnResizeMode to DynamicColumns has weird effects
        // and since we only have maximum 2 pages, settings the left page with
        // columnWidth also works.
        pageStack.columnView.columnWidth: Kirigami.Units.gridUnit * 7
        pageStack.initialPage: Kirigami.ScrollablePage {
            id: categoriesScroll
            title: i18n("Settings")
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            focus: true

            FocusScope {
                focus: true
                Accessible.role: Accessible.MenuBar
                ColumnLayout {
                    id: categories
                    Keys.onUpPressed: {
                        const buttons = categories.children

                        let foundPrevious = false
                        for (let i = buttons.length - 1; i >= 0; --i) {
                            const button = buttons[i];
                            if (!button.hasOwnProperty("highlighted")) {
                                // not a ConfigCategoryDelegate
                                continue;
                            }

                            if (foundPrevious) {
                                categories.openCategory(button.item)
                                return
                            } else if (button.highlighted) {
                                foundPrevious = true
                            }
                        }
                    }

                    Keys.onDownPressed: {
                        const buttons = categories.children

                        let foundNext = false
                        for (let i = 0, length = buttons.length; i < length; ++i) {
                            const button = buttons[i];
                            if (!button.hasOwnProperty("highlighted")) {
                                continue;
                            }

                            if (foundNext) {
                                categories.openCategory(button.item)
                                return
                            } else if (button.highlighted) {
                                foundNext = true
                            }
                        }
                    }
                    spacing: 0
                    anchors.fill: parent
                    focus: true

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
                            id: delegate
                            onActivated: categories.openCategory(model);
                            highlighted: {
                                if (model.kcm && app.pageStack.currentItem.kcm) {
                                    return model.kcm == app.pageStack.currentItem.kcm
                                }

                                if (app.pageStack.currentItem && app.pageStack.currentItem.configItem) {
                                    return model.source == app.pageStack.currentItem.configItem.source
                                }
                                return app.pageStack.currentItem.source == Qt.resolvedUrl(model.source)
                            }
                            item: model
                        }
                    }

                    Repeater {
                        Layout.fillWidth: true
                        model: root.isContainment ? globalConfigModel : undefined
                        delegate: categoryDelegate
                    }
                    Repeater {
                        Layout.fillWidth: true
                        model: configDialogFilterModel
                        delegate: categoryDelegate
                    }
                    Repeater {
                        Layout.fillWidth: true
                        model: !root.isContainment ? globalConfigModel : undefined
                        delegate: categoryDelegate
                    }
                    Repeater {
                        Layout.fillWidth: true
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

        footer: QtControls.ToolBar {
            contentItem: RowLayout {
                id: buttonsRow
                spacing: Kirigami.Units.smallSpacing

                Item {
                    Layout.fillWidth: true
                }

                QtControls.Button {
                    icon.name: "dialog-ok"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "OK")
                    onClicked: acceptAction.trigger()
                    KeyNavigation.tab: categories
                }
                QtControls.Button {
                    id: applyButton
                    enabled: false
                    icon.name: "dialog-ok-apply"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                    visible: !app.isAboutPage && app.pageStack.currentItem && (!app.pageStack.currentItem.kcm || app.pageStack.currentItem.kcm.buttons & 4) // 4 = Apply button
                    onClicked: applyAction.trigger()
                }
                QtControls.Button {
                    icon.name: "dialog-cancel"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                    onClicked: cancelAction.trigger()
                    visible: !app.isAboutPage
                }
            }
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
                app.pageStack.get(1).saveConfig()

                applyButton.enabled = false;
            }
        }

        QtControls.Action {
            id: cancelAction
            onTriggered: configDialog.close();
            shortcut: "Escape"
        }
    }
}
