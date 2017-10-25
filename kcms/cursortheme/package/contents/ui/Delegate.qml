/*
   Copyright (c) 2015 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as Controls
import QtQuick.Templates 2.2 as T2
import QtGraphicalEffects 1.0

import org.kde.kirigami 2.2 as Kirigami

import org.kde.kcm 1.0
import org.kde.private.kcm_cursortheme 1.0

T2.ItemDelegate {
    id: delegate
    onClicked: {
        view.currentIndex = index;
        view.forceActiveFocus();
    }
    width: view.cellWidth
    height: view.cellHeight
    hoverEnabled: true

    Rectangle {
        id: thumbnail
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Kirigami.Units.smallSpacing*2
        }
        height: width/1.6
        radius: Kirigami.Units.smallSpacing
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        color: {
            if (delegate.GridView.isCurrentItem) {
                return Kirigami.Theme.highlightColor;
            } else if (parent.hovered) {
                return Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.3))
            } else {
                return Kirigami.Theme.backgroundColor
            }
        }
        Behavior on color {
            ColorAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutQuad
            }
        }
        Rectangle {
            anchors {
                fill: parent
                margins: Kirigami.Units.smallSpacing * 2
            }

            color: Kirigami.Theme.backgroundColor

            PreviewWidget {
                id: previewWidget
                anchors.fill: parent
                themeModel: kcm.cursorsModel
                currentIndex: index
                //The ComboBox component is a disaster
                currentSize: parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
            }
        }

        Rectangle {
            anchors.fill: parent
            opacity: delegate.hovered || sizeMenu.visible ? 1 : 0
            radius: Kirigami.Units.smallSpacing
            color: Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 0.4)

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

            Behavior on opacity {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutQuad
                }
            }

            RowLayout {
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    margins: Kirigami.Units.smallSpacing * 2
                }
                Controls.ToolButton {
                    //TODO: use native controls' icons
                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: width
                        source: "image-resize-symbolic"
                    }
                    enabled: kcm.canResize
                    Controls.ToolTip.delay: 1000
                    Controls.ToolTip.timeout: 5000
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: i18n("Cursors Size")
                    onClicked: sizeMenu.open()

                    Controls.Menu {
                        id: sizeMenu
                        y: parent.height
                        modal: true

                        Repeater {
                            model: kcm.sizesModel
                            Controls.MenuItem {
                                text: display
                                onTriggered: {
                                    onCurrentTextChanged: {
                                        kcm.preferredSize = parseInt(display) !== NaN ? parseInt(display) : 0
                                    }
                                }
                            }
                        }
                    }
                }
                Controls.ToolButton {
                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: width
                        source: "edit-delete"
                    }
                    onClicked: kcm.removeTheme(index);
                    enabled: kcm.canRemove
                    Controls.ToolTip.delay: 1000
                    Controls.ToolTip.timeout: 5000
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: i18n("Remove Theme")
                }
            }
        }
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 2
            radius: 10
            samples: 32
            color: Qt.rgba(0, 0, 0, 0.3)
        }
    }

    Controls.Label {
        anchors {
            left: parent.left
            right: parent.right
            top: thumbnail.bottom
            topMargin: Kirigami.Units.smallSpacing
        }
        text: model.display
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }
    Controls.ToolTip.delay: 1000
    Controls.ToolTip.timeout: 5000
    Controls.ToolTip.visible: hovered
    Controls.ToolTip.text: model.description
}
