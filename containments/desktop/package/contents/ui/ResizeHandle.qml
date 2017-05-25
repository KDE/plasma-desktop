/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2015 Eike Hein <hein@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.plasmoid 2.0


MouseArea {
    width: units.gridUnit
    height: units.gridUnit
    cursorShape: Qt.SizeVerCursor
    z: 99999

    //to controls the resize/move behavior
    property bool moveX
    property bool moveY
    property bool resizeWidth
    property bool resizeHeight

    //internal
    property int startX
    property int startY
    property int startWidth
    property int startHeight

    visible: applet && applet.backgroundHints != PlasmaCore.Types.NoBackground && enabled && !plasmoid.immutable && showAppletHandle
    LayoutMirroring.enabled: false
    onPressed: {
        mouse.accepted = true
        animationsEnabled = false;
        var pos = mapToItem(root, mouse.x, mouse.y);
        startX = pos.x;
        startY = pos.y;
        startWidth = appletItem.width;
        startHeight = appletItem.height;
        appletItem.releasePosition();
        appletItem.floating = true;
        appletContainer.clip = true
    }
    onPositionChanged: {
        var pos = mapToItem(root, mouse.x, mouse.y);
        var xDelta = moveX ? pos.x - startX : startX - pos.x;
        var yDelta = moveY ? startY - pos.y : pos.y - startY;

        var oldRight = appletItem.x + appletItem.width;
        if (resizeWidth) {
            appletItem.width = Math.min(Math.max(appletItem.minimumWidth, startWidth - xDelta), appletItem.maximumWidth);
        }

        var oldBottom = appletItem.y + appletItem.height;
        if (resizeHeight) {
            appletItem.height = Math.min(Math.max(appletItem.minimumHeight, startHeight + yDelta), appletItem.maximumHeight);
        }

        if (moveX) {
            appletItem.x = oldRight - appletItem.width;
        }
        if (moveY) {
            appletItem.y = oldBottom - appletItem.height;
        }
    }
    onReleased: {
        animationsEnabled = true
        appletItem.floating = false;
        appletItem.positionItem();
        root.layoutManager.save()
        appletContainer.clip = false
    }
}
