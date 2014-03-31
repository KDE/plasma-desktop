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
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0

Item {
    id: root

    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    property var prettyStrings: {
        "LeftButton": i18n("Left-Button"),
        "RightButton": i18n("Right-Button"),
        "MidButton": i18n("Middle-Button"),

        "wheel:Vertical": i18n("Vertical-Scroll"),
        "wheel:Horizontal": i18n("Horizontal-Scroll"),

        "ShiftModifier": i18n("Shift"),
        "ControlModifier": i18n("Ctrl"),
        "AltModifier": i18n("Alt"),
        "MetaModifier": i18n("Meta")
    }

    function saveConfig() {
        configDialog.currentContainmentActionsModel.save();
    }

    Column {
        anchors {
            top: parent.top
            topMargin: 25
            horizontalCenter: parent.horizontalCenter
        }

        Repeater {
            model: configDialog.currentContainmentActionsModel
            delegate: RowLayout {
                width: root.width
                MouseEventInputButton {
                    defaultText: prettyStrings ? (prettyStrings[model.action.split(';')[1]] ? prettyStrings[model.action.split(';')[1]] + "+" : "") + prettyStrings[model.action.split(';')[0]] : ""
                    eventString: model.action
                    onEventStringChanged: {
                        configDialog.currentContainmentActionsModel.update(index, eventString, model.pluginName);
                    }
                }

                QtControls.ComboBox {
                    id: pluginsCombo
                    Layout.fillWidth: true
                    model: configDialog.containmentActionConfigModel
                    textRole: "name"
                    property bool initialized: false
                    Component.onCompleted: {
                        for (var i = 0; i < configDialog.containmentActionConfigModel.count; ++i) {
                            if (configDialog.containmentActionConfigModel.get(i).pluginName == pluginName) {
                                pluginsCombo.currentIndex = i;
                                break;
                            }
                        }
                        pluginsCombo.initialized = true;
                    }
                    onCurrentIndexChanged: {
                        if (initialized && configDialog.containmentActionConfigModel.get(currentIndex).pluginName != pluginName) {
                            configDialog.currentContainmentActionsModel.update(index, action, configDialog.containmentActionConfigModel.get(currentIndex).pluginName);
                        }
                    }
                }
                QtControls.Button {
                    iconName: "configure"
                    width: height
                    enabled: model.hasConfigurationInterface
                    onClicked: {
                        configDialog.currentContainmentActionsModel.showConfiguration(index);
                    }
                }
                QtControls.Button {
                    iconName: "dialog-information"
                    width: height
                    onClicked: {
                        configDialog.currentContainmentActionsModel.showAbout(index);
                    }
                }
                QtControls.Button {
                    iconName: "list-remove"
                    width: height
                    onClicked: {
                        configDialog.currentContainmentActionsModel.remove(index);
                    }
                }
            }
        }
        MouseEventInputButton {
            defaultText: i18n("Add Action");
            onEventStringChanged: {
                configDialog.currentContainmentActionsModel.append(eventString, "org.kde.contextmenu");
            }
        }
    }

}
