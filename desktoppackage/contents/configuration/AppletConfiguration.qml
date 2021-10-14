/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    implicitHeight: PlasmaCore.Units.gridUnit * 30

    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 30
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 21

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
        if (app.pageStack.depth === 0) {
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


    QtControls.ScrollView {
        id: categoriesScroll
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: Kirigami.Units.gridUnit * 7
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        focus: true
        Accessible.role: Accessible.MenuBar
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }

        ColumnLayout {
            id: categories

            spacing: 0
            width: categoriesScroll.width
            focus: true

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
                        if ( app.pageStack.currentItem ){
                            if (model.kcm && app.pageStack.currentItem.kcm) {
                                return model.kcm == app.pageStack.currentItem.kcm
                            } else if (app.pageStack.currentItem.configItem) {
                                return model.source == app.pageStack.currentItem.configItem.source
                            } else {
                                return app.pageStack.currentItem.source == Qt.resolvedUrl(model.source)
                            }
                        }
                        return false
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

    Kirigami.Separator {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        z: 1
    }
    Kirigami.Separator {
        anchors {
            right: categoriesScroll.right
            top: parent.top
            bottom: parent.bottom
        }
        z: 1
    }

    Kirigami.ApplicationItem {
        id: app
        anchors {
            left: categoriesScroll.right
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }

        pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Breadcrumb
        wideScreen: true
        pageStack.globalToolBar.separatorVisible: bottomSeparator.visible
        pageStack.globalToolBar.colorSet: Kirigami.Theme.Window

        property var currentConfigPage: null
        property bool isAboutPage: false

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

        footer: QtControls.Pane {

            padding: Kirigami.Units.largeSpacing

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
            background: Item {
                Kirigami.Separator {
                    id: bottomSeparator
                    visible: app.pageStack.currentItem
                        && app.pageStack.currentItem.flickable
                        && !(app.pageStack.currentItem.flickable.atYBeginning
                        && app.pageStack.currentItem.flickable.atYEnd)
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }
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
                app.pageStack.get(0).saveConfig()

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
