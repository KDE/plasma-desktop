/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore

import "./components" as Local

Item {
    id: root

    function setFocus() {
        activityName.forceActiveFocus();
        console.log("GeneralTab: Set focus called");
    }

    property string activityId: ""

    property alias activityName        : activityName.text
    property alias activityDescription : activityDescription.text
    property alias activityIcon        : buttonIcon.iconName
    property alias activityWallpaper   : imageWallpaper.source

    height : content.childrenRect.height + 4 * units.smallSpacing
    width  : content.childrenRect.width + 4 * units.smallSpacing

    Column {
        id: content

        anchors {
            fill: parent
            margins: 2 * units.smallSpacing
        }

        spacing: units.smallSpacing

        QtControls.Label {
            font.bold: true
            text: i18nd("kcm_activities5", "Activity information")
        }

        property int labelWidth : 2 * units.largeSpacing +
                Math.max(activityName.desiredLabelWidth, activityDescription.desiredLabelWidth)

        Local.LabeledTextField {
            id: activityName
            label: i18nd("kcm_activities5", "Name:")

            labelWidth: content.labelWidth
        }

        Local.LabeledTextField {
            id: activityDescription
            label: i18nd("kcm_activities5", "Description:")

            labelWidth: content.labelWidth
        }

        Item {
            width: parent.width
            height: units.smallSpacing
        }

        Row {
            height  : units.iconSizes.large * 3
            width   : childrenRect.width
            spacing : units.largeSpacing

            Item {
                id: panelWallpaper

                visible: false

                height: parent.height
                width: buttonChangeWallpaper.width + imageWallpaper.width + units.smallSpacing

                QtControls.Label {
                    id: labelWallpaper
                    font.bold: true
                    text: i18nd("kcm_activities5", "Wallpaper")
                }

                QtControls.Button {
                    id: buttonChangeWallpaper
                    width: content.labelWidth
                    text: i18ndc("kcm_activities5", "@action:button", "Change...")

                    anchors {
                        verticalCenter: imageWallpaper.verticalCenter
                    }
                }

                Image {
                    id: imageWallpaper
                    source: ""

                    width: height / 3 * 4

                    anchors {
                        top: labelWallpaper.bottom
                        bottom: parent.bottom
                        left: buttonChangeWallpaper.right

                        leftMargin: units.smallSpacing
                    }

                }
            }

            Item {
                id: panelIcon

                height : parent.height
                width  : parent.height

                QtControls.Label {
                    id: labelIcon
                    font.bold: true
                    text: i18nd("kcm_activities5", "Icon")
                }

                Item {
                    anchors {
                        top: labelIcon.bottom
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                    }

                    Local.IconChooser {
                        id: buttonIcon

                        width: height
                        height: 2 * units.iconSizes.large

                        anchors {
                            centerIn: parent
                        }
                    }
                }
            }
        }
    }
}

