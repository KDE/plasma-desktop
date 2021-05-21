/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.3 as QtControls
import QtQuick.Layouts 1.0

Item {
    id: root

    signal configurationChanged
    implicitWidth: mainColumn.implicitWidth
    implicitHeight: mainColumn.implicitHeight

    property var prettyStrings: {
        "LeftButton": i18nd("plasma_shell_org.kde.plasma.desktop", "Left-Button"),
        "RightButton": i18nd("plasma_shell_org.kde.plasma.desktop", "Right-Button"),
        "MiddleButton": i18nd("plasma_shell_org.kde.plasma.desktop", "Middle-Button"),
        "BackButton": i18nd("plasma_shell_org.kde.plasma.desktop", "Back-Button"),
        "ForwardButton": i18nd("plasma_shell_org.kde.plasma.desktop", "Forward-Button"),

        "wheel:Vertical": i18nd("plasma_shell_org.kde.plasma.desktop", "Vertical-Scroll"),
        "wheel:Horizontal": i18nd("plasma_shell_org.kde.plasma.desktop", "Horizontal-Scroll"),

        "ShiftModifier": i18nd("plasma_shell_org.kde.plasma.desktop", "Shift"),
        "ControlModifier": i18nd("plasma_shell_org.kde.plasma.desktop", "Ctrl"),
        "AltModifier": i18nd("plasma_shell_org.kde.plasma.desktop", "Alt"),
        "MetaModifier": i18nd("plasma_shell_org.kde.plasma.desktop", "Meta")
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
        y: 25
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
                        return prettyStrings[item] || item;
                    }).join(i18nc("Concatenation sign for shortcuts, e.g. Ctrl+Shift", "+"));
                }
                eventString: model.action
                onEventStringChanged: {
                    configDialog.currentContainmentActionsModel.update(index, eventString, model.pluginName);
                }
            }
        }

        Repeater {
            model: configDialog.currentContainmentActionsModel

            QtControls.ComboBox {
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
                property bool initialized: false
                Component.onCompleted: {
                    for (var i = 0; i < configDialog.containmentActionConfigModel.count; ++i) {
                        if (configDialog.containmentActionConfigModel.get(i).pluginName === pluginName) {
                            pluginsCombo.currentIndex = i;
                            break;
                        }
                    }
                    pluginsCombo.initialized = true;
                }
                onActivated: {
                    if (initialized) {
                        var newPluginName = configDialog.containmentActionConfigModel.get(index).pluginName;
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

                QtControls.Button {
                    icon.name: "configure"
                    width: height
                    enabled: model.hasConfigurationInterface
                    onClicked: {
                        configDialog.currentContainmentActionsModel.showConfiguration(index, this);
                    }
                }
                QtControls.Button {
                    icon.name: "dialog-information"
                    width: height
                    onClicked: {
                        configDialog.currentContainmentActionsModel.showAbout(index, this);
                    }
                }
                QtControls.Button {
                    icon.name: "list-remove"
                    width: height
                    onClicked: {
                        configDialog.currentContainmentActionsModel.remove(index);
                    }
                }
            }
        }

        MouseEventInputButton {
            defaultText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Action");
            onEventStringChanged: {
                configDialog.currentContainmentActionsModel.append(eventString, "org.kde.contextmenu");
            }
        }
    }

}
