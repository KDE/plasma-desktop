/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    signal configurationChanged

    property var prettyStrings: {
        "LeftButton": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Left-Button"),
        "RightButton": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Right-Button"),
        "MiddleButton": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Middle-Button"),
        "BackButton": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Back-Button"),
        "ForwardButton": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Forward-Button"),

        "wheel:Vertical": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Vertical-Scroll"),
        "wheel:Horizontal": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Horizontal-Scroll"),

        "ShiftModifier": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Shift"),
        "ControlModifier": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Ctrl"),
        "AltModifier": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Alt"),
        "MetaModifier": i18ndc("plasma_shell_org.kde.plasma.desktop", "@label for shortcuts", "Meta")
    }

    function saveConfig() {
        configDialog.currentContainmentActionsModel.save();
    }

    Connections {
        target: configDialog.currentContainmentActionsModel
        function onConfigurationChanged() {
            root.configurationChanged()
        }
    }

    GridLayout {
        id: mainColumn
        flow: GridLayout.TopToBottom
        width: parent.width

        Repeater {
            id: actionsRepeater
            model: configDialog.currentContainmentActionsModel

            MouseEventInputButton {
                Layout.column: 0
                Layout.row: index
                Layout.fillWidth: true
                Layout.minimumWidth: implicitWidth
                defaultText: {
                    var splitAction = model.action.split(';');

                    var button = splitAction[0];
                    var modifiers = (splitAction[1] || "").split('|').filter(function (item) {
                        return item !== "NoModifier";
                    });

                    var parts = modifiers;
                    modifiers.push(button);

                    return parts.map(function (item) {
                        return root.prettyStrings[item] || item;
                    }).join(i18ndc("plasma_shell_org.kde.plasma.desktop", "Concatenation sign for shortcuts, e.g. Ctrl+Shift", "+"));
                }
                eventString: model.action
                onEventStringChanged: {
                    configDialog.currentContainmentActionsModel.update(index, eventString, model.pluginName);
                }
            }
        }

        Repeater {
            model: configDialog.currentContainmentActionsModel

            QQC2.ComboBox {
                id: pluginsCombo
                // "index" argument of onActivated shadows the model index
                readonly property int pluginIndex: index
                Layout.fillWidth: true
                Layout.column: 1
                Layout.row: index
                // both MouseEventInputButton and this ComboBox have fillWidth for a uniform layout
                // however, their implicit sizes is taken into account and they compete against
                // each other for available space. By setting an insane preferredWidth we give
                // ComboBox a greater share of the available space
                Layout.preferredWidth: 9000
                model: configDialog.containmentActionConfigModel
                textRole: "name"
                valueRole: "pluginName"
                property bool initialized: false
                Component.onCompleted: {
                    currentIndex = indexOfValue(pluginName)
                    pluginsCombo.initialized = true;
                }
                onActivated: {
                    if (initialized) {
                        var newPluginName = currentValue;
                        if (newPluginName !== pluginName) {
                            configDialog.currentContainmentActionsModel.update(pluginIndex, action, newPluginName);
                        }
                    }
                }
            }
        }

        Repeater {
            model: configDialog.currentContainmentActionsModel

            RowLayout {
                Layout.column: 2
                Layout.row: index

                QQC2.Button {
                    icon.name: "configure"
                    enabled: model.hasConfigurationInterface
                    onClicked: {
                        configDialog.currentContainmentActionsModel.showConfiguration(index, this);
                    }
                }
                QQC2.Button {
                    icon.name: "list-remove"
                    onClicked: {
                        configDialog.currentContainmentActionsModel.remove(index);
                    }
                }
            }
        }

        MouseEventInputButton {
            defaultText: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button", "Add Action");
            icon.name: checked ? "input-mouse-symbolic" : "list-add"
            onEventStringChanged: {
                configDialog.currentContainmentActionsModel.append(eventString, "org.kde.contextmenu");
            }
        }
    }
}
