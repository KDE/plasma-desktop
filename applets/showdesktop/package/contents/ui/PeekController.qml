/*
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import org.kde.plasma.private.showdesktop 0.1


Controller {
    id: controller

    titleActive: i18nc("@action:button", "Stop Peeking at Desktop")
    titleInactive: i18nc("@action:button", "Peek at Desktop")

    descriptionActive: i18nc("@info:tooltip", "Moves windows back to their original positions")
    descriptionInactive: i18nc("@info:tooltip", "Temporarily reveals the Desktop by moving open windows into screen corners")

    active: showdesktop.showingDesktop

    // override
    function toggle() {
        showdesktop.toggleDesktop();
    }

    readonly property ShowDesktop showdesktop: ShowDesktop {
        id: showdesktop
    }
}
