/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *   Copyright 2014 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.activities 0.1 as Activities

FocusScope {
    id: main
    signal closed()

    function parentClosed() {
        activityBrowser.parentClosed();
    }

    //this is used to perfectly align the filter field and delegates
    property int cellWidth: theme.mSize(theme.defaultFont).width * 30
    property int spacing: 2 * units.smallSpacing

    property int minimumWidth: cellWidth + 4 * 2
    property int minimumHeight: 0

    property bool showingDialog: activityBrowser.showingDialog

    width: minimumWidth

    ActivityBrowser {
        id: activityBrowser

        onCloseRequested: main.closed()

        anchors.fill: parent
    }
}

