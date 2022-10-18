/*
    SPDX-FileCopyrightText: 2022 Jin Liu <ad.liu.jin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
    Plasma 5.26 introduced a new config entry autoFontAndSize which defaults to true.
    This means if the user customized font before (fontFamily, boldText, italicText),
    in 5.26 these settings are ignored.
 
    So we need to set autoFontAndSize=false if:
    1. Any of these 3 old entries above is set.
    2. No new entries introduced in 5.26 (autoFontAndSize, fontSize, fontWeight, fontStyleName)
       are set, so this is a config from 5.25.

    And fontWeight should be set to 75 (Font.Bold) if boldText==true.

    See https://invent.kde.org/plasma/plasma-workspace/-/merge_requests/1809
*/

const containments = desktops().concat(panels());
for (var i in containments) {
    var cont = containments[i];
    const widgets = cont.widgets();
    for (var j in widgets) {
        var widget = widgets[j];

        if (widget.type == "org.kde.plasma.digitalclock") {
            widget.currentConfigGroup = new Array('Appearance')
            if ((widget.readConfig("fontFamily", "").length > 0
                || widget.readConfig("boldText", false) 
                || widget.readConfig("italicText", false))
                &&
                (widget.readConfig("autoFontAndSize", true)
                && widget.readConfig("fontSize", 10) === 10
                && widget.readConfig("fontWeight", 50) === 50
                && widget.readConfig("fontStyleName", "").length === 0)) {
                widget.writeConfig("autoFontAndSize", false)
                if (widget.readConfig("boldText", false)) {
                    widget.writeConfig("fontWeight", 75)
                }
                // Set the font size to the largest value (72) in the font dialog,
                // so the font autofits the panel when the panel height is less
                // than 72pt. This should keep 5.25's autosize behavior for custom
                // font.
                // For panels taller than 72pt, with custom font set in 5.25, the
                // digital clock's look may still change, though. 
                widget.writeConfig("fontSize", 72)
            }
        }
    }
}
