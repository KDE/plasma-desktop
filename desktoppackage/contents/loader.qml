/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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

import QtQuick 2
import org.kde.solid 1.0 as Solid
import org.kde.plasma.platformcomponents 2.0 as Platform

Item {
    id: main

    property string shell  : "org.kde.plasma.desktop"
    property bool willing  : true
    property int  priority : 1

    // This is not needed, but allows the
    // handler to know whether its shell is loaded
    property bool loaded   : false

    default property var children
    /* This seems to crash not being able to address Solid.Device, which seems to be 
       a problem with latest Qt 5.3. Pleas re-enable only after consultation with 
      plasma-devel@kde.org */
    /*
    Solid.Devices {
        id: keyboards
        query: "IS Keyboard"
    }
    */
}

