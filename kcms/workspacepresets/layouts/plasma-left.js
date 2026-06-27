// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// Same as the default Plasma panel, but placed on the left edge of the screen instead of the
// bottom. On a vertical panel, "height" is the thickness.
clearPanels();

const panel = newPanel();
panel.location = "left";
panel.height = 2 * Math.ceil(gridUnit * 2.5 / 2);

// Like the horizontal default caps its length to a 21:9 ratio of the screen height, cap a vertical
// panel's length to a 21:9 ratio of the screen width so it is not stretched on very tall screens.
if (panel.formFactor === "vertical") {
    const geo = screenGeometry(panel.screen);
    const maximumLength = Math.ceil(geo.width * (21 / 9));
    if (geo.height > maximumLength) {
        panel.alignment = "center";
        panel.minimumLength = maximumLength;
        panel.maximumLength = maximumLength;
    }
}

panel.addWidget("org.kde.plasma.kickoff");
panel.addWidget("org.kde.plasma.pager");
panel.addWidget("org.kde.plasma.icontasks");
panel.addWidget("org.kde.plasma.marginsseparator");
panel.addWidget("org.kde.plasma.systemtray");
panel.addWidget("org.kde.plasma.digitalclock");
panel.addWidget("org.kde.plasma.showdesktop");
