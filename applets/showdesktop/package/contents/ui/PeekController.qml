/*
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import org.kde.plasma.private.showdesktop 0.1
import org.kde.plasma.plasmoid 2.0


Controller {
    id: controller

    titleInactive: i18nc("@action:button", "Peek at Desktop")
    titleActive: Plasmoid.editMode ? titleInactive : i18nc("@action:button", "Stop Peeking at Desktop")

    descriptionActive: i18nc("@info:tooltip", "Moves windows back to their original positions")
    descriptionInactive: i18nc("@info:tooltip", "Temporarily shows the desktop by moving windows away")

    active: showdesktop.showingDesktop

    // override
    function toggle() {
        showdesktop.toggleDesktop();
    }

    readonly property ShowDesktop showdesktop: ShowDesktop {
        id: showdesktop
    }
}
