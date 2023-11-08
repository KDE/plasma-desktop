/*
    SPDX-FileCopyrightText: 2023 Akseli Lahtinen <akselmo@akselmo.dev>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Find all depicted widgets and migrate their font weights from qt5 to qt6 style

var containments = desktops().concat(panels());
for (var c in containments) {
    const cont = containments[c];
    const widgets = cont.widgets();
    for (var w in widgets) {
        var widget = widgets[w];
        switch(widget.type) {
            case "org.kde.plasma.digitalclock":
                widget.currentConfigGroup = ['Appearance'];
                // Use "normal" weight as default if weight is not set
                const oldFontWeight = widget.readConfig("fontWeight", 400);
                const newFontWeight = migrateFontWeight(Number(oldFontWeight));
                widget.writeConfig("fontWeight", newFontWeight);
                break;
        }
    }
}

function migrateFontWeight(oldWeight) {
    // Takes old weight (Qt5 weight) and returns the Qt6 equivalent
    // Qt5 font weights: https://doc.qt.io/qt-5/qfont.html#Weight-enum
    // Qt6 font weights: https://doc.qt.io/qt-6/qfont.html#Weight-enum
    var newWeight = 400;
    if (oldWeight === 0) { newWeight = 100; }
    else if (oldWeight === 12) { newWeight = 200; }
    else if (oldWeight === 25) { newWeight = 300; }
    else if (oldWeight === 50) { newWeight = 400; }
    else if (oldWeight === 57) { newWeight = 500; }
    else if (oldWeight === 63) { newWeight = 600; }
    else if (oldWeight === 75) { newWeight = 700; }
    else if (oldWeight === 81) { newWeight = 800; }
    else if (oldWeight === 87) { newWeight = 900; }

    return newWeight;
}

