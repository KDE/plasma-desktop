/********************************************************************
Copyright 2017 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.1 as KCM

Item {

    Controls.TabBar {
        id: tabbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        Controls.TabButton {
            text: i18n("Hardware")
        }
        Controls.TabButton {
            text: i18n("Layouts")
        }
        Controls.TabButton {
            text: i18n("Advanced")
        }
    }

    StackLayout {
        anchors.top: tabbar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: tabbar.currentIndex

        Hardware {}

        Layouts {}

        Advanced {}
    }
}
