/*
    Copyright 2019  Nicolas Fella <nicolas.fella@gmx.de>
    Copyright 2020  Carson Black <uhhadd@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.6
import QtQuick.Dialogs 1.1 as Dialogs
import QtGraphicalEffects 1.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.2 as KCM
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.plasma.kcm.users 1.0

KCM.ScrollViewKCM {
    id: root

    title: i18n("Manage Users")
    // Make the first page a sidebar
    Kirigami.ColumnView.fillWidth: false
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: Kirigami.Units.gridUnit * 20

    // QML cannot update avatar image when override. By increasing this number and
    // appending it to image source with '?', we force avatar to reload
    property int avatarVersion: 0

    Connections {
        target: kcm
        onApply: {
            avatarVersion += 1
        }
    }

    Component.onCompleted: {
        kcm.columnWidth = Kirigami.Units.gridUnit * 15
        kcm.push("UserDetailsPage.qml", {user: kcm.userModel.getLoggedInUser()})
    }

    view: ListView {
        id: userList
        model: kcm.userModel

        section {
            property: "sectionHeader"
            delegate: Kirigami.ListSectionHeader {
                label: section
            }
        }

        delegate: Kirigami.BasicListItem {
            text: model.display
            subtitle: model.name

            highlighted: index == userList.currentIndex

            onClicked: {
                userList.currentIndex = index
                kcm.pop(0)
                kcm.push("UserDetailsPage.qml", {user: userObject})
            }

            Kirigami.Theme.colorSet: highlighted ? Kirigami.Theme.Selection : Kirigami.Theme.View

            leading: Rectangle {
                width: height

                color: "transparent"
                border.color: Kirigami.ColorUtils.adjustColor(Kirigami.Theme.textColor, {alpha: 0.4*255})
                border.width: 1
                radius: height/2

                Kirigami.Avatar {
                    source: model.decoration + '?' + avatarVersion // force reload after saving
                    cache: false // avoid caching
                    name: model.display
                    anchors {
                        fill: parent
                        margins: 1
                    }
                }
            }
        }
    }

    footer: RowLayout {
        QQC2.Button {
            icon.name: "list-add"

            text: i18n("Add New User")

            onClicked: {
                kcm.pop(0)
                kcm.push("CreateUser.qml")
                userList.currentIndex = -1
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    Item {} // For some reason, this being here keeps the layout from breaking.
}
