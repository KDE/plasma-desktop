/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13 as QQC2
import org.kde.kirigami 2.13 as Kirigami

Kirigami.ScrollablePage {
    id: root
    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false
    padding: 0
    
    header: Kirigami.Separator {
        anchors {
            left: root.left
            right: root.right
            top: root.top
        }
    }
}
