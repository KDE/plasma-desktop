/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2022-2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.plasma.configuration 2.0
import org.kde.plasma.plasmoid 2.0

Rectangle {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 30

    Layout.minimumWidth: Kirigami.Units.gridUnit * 30
    Layout.minimumHeight: Kirigami.Units.gridUnit * 21

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    color: Kirigami.Theme.backgroundColor

    property bool isContainment: false

    property ConfigModel globalConfigModel:  globalAppletConfigModel

    property url currentSource

    function closing() {
        if (applyButton.enabled) {
            messageDialog.item = null;
            messageDialog.open();
            return false;
        }
        return true;
    }

    function saveConfig() {
        const config = Plasmoid.configuration; // type: KConfigPropertyMap

        config.keys().forEach(key => {
            const cfgKey = "cfg_" + key;
            if (cfgKey in app.pageStack.currentItem) {
                config[key] = app.pageStack.currentItem[cfgKey];
            }
        })

        plasmoid.configuration.writeConfig();

        // For ConfigurationContainmentActions.qml
        if (app.pageStack.currentItem.hasOwnProperty("saveConfig")) {
            app.pageStack.currentItem.saveConfig()
        }
    }

    Connections {
        target: configDialog
        function onClosing(event) {
            event.accepted = closing();
        }
    }

    ConfigModel {
        id: globalAppletConfigModel
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Keyboard Shortcuts")
            icon: "preferences-desktop-keyboard"
            source: Qt.resolvedUrl("ConfigurationShortcuts.qml")
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: configDialogFilterModel
        sourceModel: configDialog.configModel
        filterRowCallback: (row, parent) => {
            return sourceModel.data(sourceModel.index(row, 0), ConfigModel.VisibleRole);
        }
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
        root.currentSource = item.source

        if (item.source) {
            app.isAboutPage = item.source === Qt.resolvedUrl("AboutPlugin.qml");

            if (isContainment) {
                pushReplace(Qt.resolvedUrl("ConfigurationAppletPage.qml"), {configItem: item});
            } else {

                const config = Plasmoid.configuration; // type: KConfigPropertyMap

                const props = {
                    "title": item.name
                };

                config.keys().forEach(key => {
                    props["cfg_" + key] = config[key];
                });

                pushReplace(item.source, props);
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

    Connections {
        target: app.pageStack

        function onCurrentItemChanged() {
            if (app.pageStack.currentItem !== null && !isContainment) {
                const config = Plasmoid.configuration; // type: KConfigPropertyMap

                config.keys().forEach(key => {
                    const changedSignal = app.pageStack.currentItem["cfg_" + key + "Changed"];
                    if (changedSignal) {
                        changedSignal.connect(() => root.settingValueChanged());
                    }
                });

                const configurationChangedSignal = app.pageStack.currentItem.configurationChanged;
                if (configurationChangedSignal) {
                    configurationChangedSignal.connect(() => root.settingValueChanged());
                }
            }
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


    QQC2.ScrollView {
        id: categoriesScroll
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: Kirigami.Units.gridUnit * 7
        contentWidth: availableWidth
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        activeFocusOnTab: true
        focus: true
        Accessible.role: Accessible.PageTabList
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }

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
                    categoriesScroll.forceActiveFocus(Qt.TabFocusReason)
                    return
                } else if (button.highlighted) {
                    foundPrevious = true
                }
            }

            event.accepted = false
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
                    categoriesScroll.forceActiveFocus(Qt.TabFocusReason)
                    return
                } else if (button.highlighted) {
                    foundNext = true
                }
            }

            event.accepted = false
        }

        ColumnLayout {
            id: categories

            spacing: 0
            width: categoriesScroll.contentWidth
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
                        if (app.pageStack.currentItem) {
                            if (model.kcm && app.pageStack.currentItem.kcm) {
                                return model.kcm == app.pageStack.currentItem.kcm
                            } else {
                                return root.currentSource == model.source
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
                        source: Qt.resolvedUrl("AboutPlugin.qml")
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
        id: verticalSeparator
        anchors {
            top: parent.top
            left: categoriesScroll.right
            bottom: parent.bottom
        }
        z: 1
    }

    Kirigami.ApplicationItem {
        id: app
        anchors {
            top: parent.top
            left: verticalSeparator.right
            right: parent.right
            bottom: parent.bottom
        }

        pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Breadcrumb
        wideScreen: true
        pageStack.globalToolBar.separatorVisible: bottomSeparator.visible
        pageStack.globalToolBar.colorSet: Kirigami.Theme.Window

        property var currentConfigPage: null
        property bool isAboutPage: false

        Kirigami.PromptDialog {
            id: messageDialog
            property var item
            title: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply Settings")
            subtitle: i18nd("plasma_shell_org.kde.plasma.desktop", "The settings of the current module have changed. Do you want to apply the changes or discard them?")
            standardButtons: Kirigami.Dialog.Apply | Kirigami.Dialog.Discard | Kirigami.Dialog.Cancel
            onApplied: {
                applyAction.trigger()
                discarded();
            }
            onDiscarded: {
                if (item) {
                    root.open(item);
                    messageDialog.close();
                } else {
                    applyButton.enabled = false;
                    configDialog.close();
                }
            }
        }

        footer: QQC2.Pane {

            padding: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                id: buttonsRow
                spacing: Kirigami.Units.smallSpacing

                Item {
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    icon.name: "dialog-ok"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "OK")
                    onClicked: acceptAction.trigger()
                }
                QQC2.Button {
                    id: applyButton
                    enabled: false
                    icon.name: "dialog-ok-apply"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                    visible: !app.isAboutPage && app.pageStack.currentItem && (!app.pageStack.currentItem.kcm || app.pageStack.currentItem.kcm.buttons & 4) // 4 = Apply button
                    onClicked: applyAction.trigger()
                }
                QQC2.Button {
                    icon.name: "dialog-cancel"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                    onClicked: cancelAction.trigger()
                    visible: !app.isAboutPage
                    KeyNavigation.tab: categories
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

        QQC2.Action {
            id: acceptAction
            onTriggered: {
                applyAction.trigger();
                configDialog.close();
            }
        }

        QQC2.Action {
            id: applyAction
            onTriggered: {
                if (isContainment) {
                    app.pageStack.get(0).saveConfig()
                } else {
                    root.saveConfig()
                }

                applyButton.enabled = false;
            }
        }

        QQC2.Action {
            id: cancelAction
            onTriggered: {
                if (root.closing()) {
                    configDialog.close();
                }
            }
        }

        Keys.onReturnPressed: acceptAction.trigger();
        Keys.onEscapePressed: cancelAction.trigger();
    }
}
