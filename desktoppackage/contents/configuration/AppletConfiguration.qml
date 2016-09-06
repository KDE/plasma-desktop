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

import QtQuick 2.0
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.3 as QtControls
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2

import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0


//TODO: all of this will be done with desktop components
Rectangle {
    id: root
    Layout.minimumWidth: units.gridUnit * 30
    Layout.minimumHeight: units.gridUnit * 20

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

//BEGIN properties
    color: syspal.window
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
        if (main.currentItem.saveConfig) {
            main.currentItem.saveConfig()
        }
        for (var key in plasmoid.configuration) {
            if (main.currentItem["cfg_"+key] !== undefined) {
                plasmoid.configuration[key] = main.currentItem["cfg_"+key]
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
                main.sourceFile = configDialog.configModel.get(0).source
            } else if (configDialog.configModel.get(0).kcm) {
                main.sourceFile = Qt.resolvedUrl("ConfigurationKcmPage.qml");
                main.currentItem.kcm = configDialog.configModel.get(0).kcm;
            } else {
                main.sourceFile = "";
            }
            main.title = configDialog.configModel.get(0).name
        } else {
            main.sourceFile = globalConfigModel.get(0).source
            main.title = globalConfigModel.get(0).name
        }
//         root.width = mainColumn.implicitWidth
//         root.height = mainColumn.implicitHeight
    }
//END connections

//BEGIN UI components
    SystemPalette {id: syspal}

    MouseEventListener {
        anchors.fill: parent
        property int oldX
        property int oldY
        onPressed: {
            oldX = mouse.screenX
            oldY = mouse.screenY
        }
        onPositionChanged: {
            configDialog.y += mouse.screenY - oldY
            configDialog.x += mouse.screenX - oldX
            oldX = mouse.screenX
            oldY = mouse.screenY
        }
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

    ColumnLayout {
        id: mainColumn
        anchors {
            fill: parent
            margins: mainColumn.spacing * units.devicePixelRatio //margins are hardcoded in QStyle we should match that here
        }
        property int implicitWidth: Math.max(contentRow.implicitWidth, buttonsRow.implicitWidth) + 8
        property int implicitHeight: contentRow.implicitHeight + buttonsRow.implicitHeight + 8

        RowLayout {
            id: contentRow
            anchors {
                left: parent.left
                right: parent.right
            }
            spacing: 0
            Layout.fillHeight: true
            Layout.preferredHeight: parent.height - buttonsRow.height

            QtControls.ScrollView {
                id: categoriesScroll
                frameVisible: false
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                Layout.fillHeight: true
                visible: (configDialog.configModel ? configDialog.configModel.count : 0) + globalConfigModel.count > 1
                Layout.maximumWidth: units.gridUnit * 7
                Layout.preferredWidth: categories.implicitWidth
                flickableItem.interactive: false

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
                    width: categoriesScroll.viewport.width

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
                color: syspal.highlight
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

                // this horrible code below ensures the control with active focus stays visible in the window
                // by scrolling the view up or down as needed when tabbing through the window
                Window.onActiveFocusItemChanged: {
                    var flickable = scroll.flickableItem;

                    var item = Window.activeFocusItem;
                    if (!item) {
                        return;
                    }

                    // when an item withing ScrollView has active focus the ScrollView,
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

                Column {
                    spacing: units.largeSpacing / 2

                    QtControls.Label {
                        id: pageTitle
                        width: scroll.viewport.width
                        font.pointSize: theme.defaultFont.pointSize*2
                        font.weight: Font.Light
                        text: main.title
                    }

                    QtControls.StackView {
                        id: main
                        property string title: ""
                        property bool invertAnimations: false

                        height: Math.max((scroll.viewport.height - pageTitle.height - parent.spacing), (main.currentItem  ? (main.currentItem.implicitHeight ? main.currentItem.implicitHeight : main.currentItem.childrenRect.height) : 0))
                        width: scroll.viewport.width

                        property string sourceFile

                        onSourceFileChanged: {
                            if (!sourceFile) {
                                return;
                            }

                            var props = {}

                            var plasmoidConfig = plasmoid.configuration
                            for (var key in plasmoidConfig) {
                                props["cfg_" + key] = plasmoid.configuration[key]
                            }

                            var newItem = push({
                                item: Qt.resolvedUrl(sourceFile),
                                replace: true,
                                properties: props
                            })

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

                        delegate: QtControls.StackViewDelegate {
                            function transitionFinished(properties)
                            {
                                properties.exitItem.opacity = 1
                            }

                            pushTransition: QtControls.StackViewTransition {
                                PropertyAnimation {
                                    target: enterItem
                                    property: "opacity"
                                    from: 0
                                    to: 1
                                    duration: units.longDuration
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    target: enterItem
                                    property: "x"
                                    from: main.invertAnimations ? -target.width/3: target.width/3
                                    to: 0
                                    duration: units.longDuration
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    target: exitItem
                                    property: "opacity"
                                    from: 1
                                    to: 0
                                    duration: units.longDuration
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    target: exitItem
                                    property: "x"
                                    from: 0
                                    to: main.invertAnimations ? target.width/3 : -target.width/3
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
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            QtControls.Button {
                iconName: "dialog-ok"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "OK")
                onClicked: acceptAction.trigger()
            }
            QtControls.Button {
                id: applyButton
                enabled: false
                iconName: "dialog-ok-apply"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                visible: main.currentItem && (!main.currentItem.kcm || main.currentItem.kcm.buttons & 4) // 4 = Apply button
                onClicked: applyAction.trigger()
            }
            QtControls.Button {
                iconName: "dialog-cancel"
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                onClicked: cancelAction.trigger()
            }
        }
    }
//END UI components
}
