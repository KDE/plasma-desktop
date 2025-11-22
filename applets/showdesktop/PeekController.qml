/*
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import plasma.applet.org.kde.plasma.showdesktop
import org.kde.plasma.plasmoid


Controller {
    id: controller

    titleInactive: i18nc("@action:button", "Peek at Desktop")  // qmllint disable unqualified
    titleActive: Plasmoid.containment.corona.editMode ? titleInactive : i18nc("@action:button", "Stop Peeking at Desktop")  // qmllint disable unqualified

    descriptionActive: i18nc("@info:tooltip", "Moves windows back to their original positions")  // qmllint disable unqualified
    descriptionInactive: i18nc("@info:tooltip", "Temporarily shows the desktop by moving windows away")  // qmllint disable unqualified

    active: showdesktop.showingDesktop

    // override
    function toggle() {
        showdesktop.toggleDesktop();
    }

    readonly property ShowDesktop showdesktop: ShowDesktop {
        id: showdesktop
    }
}
