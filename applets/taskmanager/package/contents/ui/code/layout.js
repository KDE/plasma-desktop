/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.pragma library

.import org.kde.kirigami 2.20 as Kirigami

const iconMargin = Math.round(Kirigami.Units.smallSpacing / 4);
const labelMargin = Kirigami.Units.smallSpacing;

// Returns the number of 'm' characters whose joint width must be available in the task button label
// so that the button text is rendered at all.
function minimumMColumns(vertical) {
    return vertical ? 4 : 5;
}
