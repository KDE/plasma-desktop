/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/**
    @c showSeconds option now supports showing seconds only in the tooltip.

    @see https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/2232
    @since 6.0
*/

const containments = desktops().concat(panels());

containments.forEach(containment => containment.widgets("org.kde.plasma.digitalclock").forEach(widget => {
    widget.currentConfigGroup = ["Appearance"];
    if (widget.readConfig("showSeconds", false /* Default: never show seconds */) === true /* Changed by the user */) {
        widget.writeConfig("showSeconds", 2 /* Always show seconds */);
    }
}));
