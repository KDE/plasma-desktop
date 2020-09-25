/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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


//TODO: all of this will be done with desktop components
Rectangle {
    id: root
    Layout.minimumWidth: units.gridUnit * 30
    Layout.minimumHeight: units.gridUnit * 20

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

//BEGIN properties
    color: Kirigami.Theme.backgroundColor
    width: units.gridUnit * 40
    height: units.gridUnit * 30

    property bool isContainment: false
//END properties

//BEGIN model
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
//END model

//BEGIN functions
    function saveConfig() {
        if (pageStack.currentItem.saveConfig) {
            pageStack.currentItem.saveConfig()
        }
        for (var key in plasmoid.configuration) {
            if (pageStack.currentItem["cfg_"+key] !== undefined) {
                plasmoid.configuration[key] = pageStack.currentItem["cfg_"+key]
            }
        }
    }

    function settingValueChanged() {
        applyButton.enabled = true;
    }
//END functions


//BEGIN connections
    Component.onCompleted: {
        if (!isContainment && configDialog.configModel && configDialog.configModel.count > 0) {
            if (configDialog.configModel.get(0).source) {
                pageStack.sourceFile = configDialog.configModel.get(0).source
            } else if (configDialog.configModel.get(0).kcm) {
                pageStack.sourceFile = Qt.resolvedUrl("ConfigurationKcmPage.qml");
                pageStack.currentItem.kcm = configDialog.configModel.get(0).kcm;
            } else {
                pageStack.sourceFile = "";
            }
            pageStack.title = configDialog.configModel.get(0).name
        } else {
            pageStack.sourceFile = globalConfigModel.get(0).source
            pageStack.title = globalConfigModel.get(0).name
        }
    }
//END connections

//BEGIN UI components
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
    }

    Kirigami.Separator {
        id: topSeparator
        anchors.top: root.top
        width: root.width
    }

    MessageDialog {
        id: messageDialog
        icon: StandardIcon.Warning
        property Item delegate
        title: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply Settings")
        text: i18nd("plasma_shell_org.kde.plasma.desktop", "The settings of the current module have changed. Do you want to apply the changes or discard them?")
        standardButtons: StandardButton.Apply | StandardButton.Discard | StandardButton.Cancel
        onApply: {
            applyAction.trigger()
            delegate.openCategory()
        }
        onDiscard: {
            delegate.openCategory()
        }
    }

    RowLayout {
        anchors {
            topMargin: topSeparator.height
            fill: parent
        }
        spacing: 0

        QtControls.ScrollView {
            id: categoriesScroll
            Layout.fillHeight: true
            visible: true
            clip: true
            Layout.preferredWidth: units.gridUnit * 7
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
                        button.openCategory()
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
                    console.log(button)
                    if (!button.hasOwnProperty("current")) {
                        continue;
                    }

                    if (foundNext) {
                        button.openCategory()
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

                Repeater {
                    model: root.isContainment ? globalConfigModel : undefined
                    delegate: ConfigCategoryDelegate {}
                }
                Repeater {
                    model: configDialogFilterModel
                    delegate: ConfigCategoryDelegate {}
                }
                Repeater {
                    model: !root.isContainment ? globalConfigModel : undefined
                    delegate: ConfigCategoryDelegate {}
                }
                 Repeater {
                    model: ConfigModel {
                        ConfigCategory{
                            name: i18nd("plasma_shell_org.kde.plasma.desktop", "About")
                            icon: "help-about"
                            source: "AboutPlugin.qml"
                        }
                    }
                    delegate: ConfigCategoryDelegate {}
                }
            }
        }

        // Configuration area and buttons.
        ColumnLayout {
            id: configColumn
            Layout.topMargin: topSeparator.height
            Layout.bottomMargin: units.smallSpacing * 2

            // Configuration scroll area
            QtControls.ScrollView {
                id: scroll
                Layout.fillHeight: true
                Layout.fillWidth: true
                // we want to focus the controls in the settings page right away, don't focus the ScrollView
                activeFocusOnTab: false
                // Avoid scrollbar flashing on/off when decrease the window height, that is created by the content matching the scroll height.
                // Even if scrollbar does not appear in the UI, modifies the availableWidth causing other issues.
                QtControls.ScrollBar.vertical.policy: pageStack.maxHeight > pageStack.contentHeight ? QtControls.ScrollBar.AlwaysOff : QtControls.ScrollBar.AlwaysOn

                property Item flickableItem: pageFlickable
                // this horrible code below ensures the control with active focus stays visible in the window
                // by scrolling the view up or down as needed when tabbing through the window
                Window.onActiveFocusItemChanged: {
                    var flickable = scroll.flickableItem;

                    var item = Window.activeFocusItem;
                    if (!item) {
                        return;
                    }

                    // when an item within ScrollView has active focus the ScrollView,
                    // as FocusScope, also has it, so we only scroll in this case
                    if (!scroll.activeFocus) {
                        return;
                    }

                    var padding = units.gridUnit * 2 // some padding to the top/bottom when we scroll

                    var yPos = item.mapToItem(scroll.contentItem, 0, 0).y;
                    if (yPos < flickable.contentY) {
                        flickable.contentY = Math.max(0, yPos - padding);

                    // The "Math.min(padding, item.height)" ensures that we only scroll the item into view
                    // when it's barely visible. The logic was mostly meant for keyboard navigating through
                    // a list of CheckBoxes, so this check keeps us from trying to scroll an inner ScrollView
                    // into view when it implicitly gains focus (like plasma-pa config dialog has).
                    } else if (yPos + Math.min(padding, item.height) > flickable.contentY + flickable.height) {
                        flickable.contentY = Math.min(flickable.contentHeight - flickable.height,
                                                      yPos - flickable.height + item.height + padding);
                    }
                }
                Flickable {
                    id: pageFlickable

                    anchors {
                        top: scroll.top
                        bottom: scroll.bottom
                        left: scroll.left
                    }
                    width: scroll.availableWidth
                    contentHeight: pageColumn.height
                    contentWidth: width

                    Column {
                        id: pageColumn
                        spacing: units.largeSpacing / 2
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: units.smallSpacing * 2
                            rightMargin: units.smallSpacing * 2
                        }

                        Kirigami.Heading {
                            id: pageTitle
                            width: pageColumn.width
                            topPadding: units.smallSpacing
                            level: 1
                            text: pageStack.title
                        }

                        QtControls.StackView {
                            id: pageStack
                            property string title: ""
                            property bool invertAnimations: false

                            property var maxHeight: scroll.availableHeight - pageTitle.height - parent.spacing
                            property var contentHeight: pageStack.currentItem ? (pageStack.currentItem.implicitHeight
                                                                                 ? pageStack.currentItem.implicitHeight
                                                                                 : pageStack.currentItem.childrenRect.height) : 0
                            height: Math.max(maxHeight, contentHeight)
                            width: pageColumn.width

                            property string sourceFile

                            onSourceFileChanged: {
                                if (!sourceFile) {
                                    return;
                                }

                                //in a StackView pages need to be initialized with stackviews size, or have none
                                var props = {"width": width, "height": height}

                                var plasmoidConfig = plasmoid.configuration
                                for (var key in plasmoidConfig) {
                                    props["cfg_" + key] = plasmoid.configuration[key]
                                }

                                var newItem = replace(Qt.resolvedUrl(sourceFile), props)

                                for (var key in plasmoidConfig) {
                                    var changedSignal = newItem["cfg_" + key + "Changed"]
                                    if (changedSignal) {
                                        changedSignal.connect(root.settingValueChanged)
                                    }
                                }

                                var configurationChangedSignal = newItem.configurationChanged
                                if (configurationChangedSignal) {
                                    configurationChangedSignal.connect(root.settingValueChanged)
                                }

                                applyButton.enabled = false;
                                scroll.flickableItem.contentY = 0
                                /*
                                    * This is not needed on a desktop shell that has ok/apply/cancel buttons, i'll leave it here only for future reference until we have a prototype for the active shell.
                                    * root.pageChanged will start a timer, that in turn will call saveConfig() when triggered

                                for (var prop in currentItem) {
                                    if (prop.indexOf("cfg_") === 0) {
                                        currentItem[prop+"Changed"].connect(root.pageChanged)
                                    }
                                }*/
                            }

                            replaceEnter: Transition {
                                ParallelAnimation {
                                    //OpacityAnimator when starting from 0 is buggy (it shows one frame with opacity 1)
                                    NumberAnimation {
                                        property: "opacity"
                                        from: 0
                                        to: 1
                                        duration: units.longDuration
                                        easing.type: Easing.InOutQuad
                                    }
                                    XAnimator {
                                        from: pageStack.invertAnimations ? -pageColumn.width/3: pageColumn.width/3
                                        to: 0
                                        duration: units.longDuration
                                        easing.type: Easing.InOutQuad
                                    }
                                }
                            }
                            replaceExit: Transition {
                                ParallelAnimation {
                                    OpacityAnimator {
                                        from: 1
                                        to: 0
                                        duration: units.longDuration
                                        easing.type: Easing.InOutQuad
                                    }
                                    XAnimator {
                                        from: 0
                                        to: pageStack.invertAnimations ? pageColumn.width/3 : -pageColumn.width/3
                                        duration: units.longDuration
                                        easing.type: Easing.InOutQuad
                                    }
                                }
                            }
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
                    root.saveConfig();

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
                Layout.rightMargin: units.smallSpacing * 2
                Layout.leftMargin: units.smallSpacing * 2

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
                    visible: pageStack.currentItem && (!pageStack.currentItem.kcm || pageStack.currentItem.kcm.buttons & 4) // 4 = Apply button
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
//END UI components
}
