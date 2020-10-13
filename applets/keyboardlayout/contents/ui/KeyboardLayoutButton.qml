/***************************************************************************
 *   Copyright (C) 2014 by Daniel Vrátil <dvratil@redhat.com>              *
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

import QtQuick 2.1
import QtQuick.Controls 1.1 as QQC

import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.plasma.workspace.keyboardlayout 1.0



PlasmaComponents3.ToolButton {

    property int fontSize: config.fontSize

    id: kbLayoutButton

    property alias button: kbLayoutButton

    icon.name: "input-keyboard"
    text: layout.currentLayoutDisplayName
    font.pointSize: Math.max(fontSize,theme.defaultFont.pointSize)

    Accessible.name: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Button to change keyboard layout", "Switch layout")

    visible: layout.layouts.length > 1

    onClicked: layout.nextLayout()

    KeyboardLayout {
          id: layout
              function nextLayout() {
              var layouts = layout.layouts;
              var index = (layouts.indexOf(layout.currentLayout)+1) % layouts.length;
              layout.currentLayout = layouts[index];
          }
    }
}
