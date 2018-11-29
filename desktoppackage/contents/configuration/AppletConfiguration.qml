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
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Keyboard shortcuts")
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
//         root.width = pageStackColumn.implicitWidth
//         root.height = pageStackColumn.implicitHeight
    }
//END connections

//BEGIN UI components
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

    ColumnLayout {
        id: pageStackColumn
        anchors {
            fill: parent
            margins: pageStackColumn.spacing * units.devicePixelRatio //margins are hardcoded in QStyle we should match that here
        }
        property int implicitWidth: Math.max(contentRow.implicitWidth, buttonsRow.implicitWidth) + 8
        property int implicitHeight: contentRow.implicitHeight + buttonsRow.implicitHeight + 8

        RowLayout {
            id: contentRow
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height - buttonsRow.height

            QtControls.ScrollView {
                id: categoriesScroll
                QtControls.ScrollBar.horizontal.policy: QtControls.ScrollBar.AlwaysOff
                Layout.fillHeight: true
                visible: (configDialog.configModel ? configDialog.configModel.count : 0) + globalConfigModel.count > 1
                Layout.maximumWidth: units.gridUnit * 7
                Layout.preferredWidth: categories.implicitWidth

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
                    width: categoriesScroll.width

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
                }
            }

            Rectangle {
                Layout.fillHeight: true
                width: Math.round(units.devicePixelRatio)
                color: Kirigami.Theme.highlightColor
                visible: categoriesScroll.visible
                opacity: categoriesScroll.activeFocus && Window.active ? 1 : 0.3
                Behavior on color {
                    ColorAnimation {
                        duration: units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Item { // spacer
                width: units.largeSpacing
                visible: categoriesScroll.visible
            }

            QtControls.ScrollView {
                id: scroll
                Layout.fillHeight: true
                Layout.fillWidth: true
                // we want to focus the controls in the settings page right away, don't focus the ScrollView
                activeFocusOnTab: false

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
                    anchors.fill: parent
                    contentHeight: pageColumn.height
                    contentWidth: width
                    Column {
                        id: pageColumn
                        spacing: units.largeSpacing / 2

                        Kirigami.Heading {
                            id: pageTitle
                            width: scroll.width
                            level: 1
                            text: pageStack.title
                        }

                        QtControls.StackView {
                            id: pageStack
                            property string title: ""
                            property bool invertAnimations: false

                            height: Math.max((scroll.height - pageTitle.height - parent.spacing), (pageStack.currentItem  ? (pageStack.currentItem.implicitHeight ? pageStack.currentItem.implicitHeight : pageStack.currentItem.childrenRect.height) : 0))
                            width: scroll.width

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
                                        from: pageStack.invertAnimations ? -scroll.width/3: scroll.width/3
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
                                        to: pageStack.invertAnimations ? scroll.width/3 : -scroll.width/3
                                        duration: units.longDuration
                                        easing.type: Easing.InOutQuad
                                    }
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
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
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
//END UI components
}
