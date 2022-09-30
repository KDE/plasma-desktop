/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/**
    The Media Frame widget removes the useBackground option and uses ConfigurableBackground
    hint to support toggling the background directly from the widget toolbar.

    The option only applies to media frame widgets on desktop.

    @see https://invent.kde.org/plasma/kdeplasma-addons/-/merge_requests/238
    @since 5.27
*/
desktops().forEach(containment => containment.widgets("org.kde.plasma.mediaframe").forEach(widget => {
    widget.currentConfigGroup = ["General"];
    if (widget.readConfig("useBackground", true /* Default */) === false /* Changed by the user */) {
        widget.writeConfig("useBackground", "");
        widget.currentConfigGroup = []; // Root Configuration
        widget.writeConfig("UserBackgroundHints", "NoBackground");
    }
}));
