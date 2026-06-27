// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// KDE Plasma default: a single Task Manager panel along the bottom edge, matching the
// out-of-the-box default panel, including its 21:9 maximum width on wide screens.
clearPanels();

const panel = newPanel();
panel.location = "bottom";

// For an Icons-Only Task Manager on the bottom, *3 is too much, *2 is too little. Round up to the
// next even number, like the default layout (the panel size widget only shows even numbers).
panel.height = 2 * Math.ceil(gridUnit * 2.5 / 2);

// Restrict a horizontal panel to a maximum size of a 21:9 monitor, like the default layout, so it
// is not stretched to the full width of an ultra-wide screen.
if (panel.formFactor === "horizontal") {
    const geo = screenGeometry(panel.screen);
    const maximumWidth = Math.ceil(geo.height * (21 / 9));
    if (geo.width > maximumWidth) {
        panel.alignment = "center";
        panel.minimumLength = maximumWidth;
        panel.maximumLength = maximumWidth;
    }
}

panel.addWidget("org.kde.plasma.kickoff");
panel.addWidget("org.kde.plasma.pager");
panel.addWidget("org.kde.plasma.icontasks");
panel.addWidget("org.kde.plasma.marginsseparator");
panel.addWidget("org.kde.plasma.systemtray");
panel.addWidget("org.kde.plasma.digitalclock");
panel.addWidget("org.kde.plasma.showdesktop");
