/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 2.4
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.4 as Kirigami

import org.kde.plasma.plasmoid 2.0

Item {
    id: configExperimental

    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_showToolbox: showToolbox.checked
    property alias cfg_pressToMove: pressToMove.checked

    property bool showMessage: false

    ColumnLayout {

        Kirigami.InlineMessage {
            visible: showMessage || animating
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "With the Desktop Toolbox hidden, right-click on the desktop and choose 'Configure Desktop...' to access this configuration window in the future")
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
        }

        Kirigami.FormLayout {

            CheckBox {
                id: showToolbox

                text: i18n("Show the desktop toolbox")

                // Only show the message when the user actively unchecks it, not if it's
                // already unchecked
                nextCheckState: function() {
                    if (checkState === Qt.Checked) {
                        showMessage = true
                        return Qt.Unhecked
                    } else {
                        showMessage = false
                        return Qt.Checked
                    }
                }
            }

            CheckBox {
                id: pressToMove

                text: i18n("Press and hold widgets to move them and reveal their handles")
            }
        }
    }
}
