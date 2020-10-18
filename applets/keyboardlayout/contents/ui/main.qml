/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.12
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root

    Plasmoid.icon: "input-keyboard"

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: CompactRepresentation {}

    // You must have more than one keyboard layout in order to see the applet
    Plasmoid.status: Plasmoid.compactRepresentation.hasMoreThanOneKeyboardLayout ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    Plasmoid.toolTipSubText: "Toggle the keyboard layout"
}
