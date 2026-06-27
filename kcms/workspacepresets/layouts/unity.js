// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// Unity-like: a vertical launcher dock on the left edge and a top bar with the global menu and status area.
clearPanels();

const launcher = newPanel();
launcher.location = "left";
// On a vertical panel, "height" is the thickness (the horizontal extent).
launcher.height = Math.ceil(gridUnit * 3);
launcher.addWidget("org.kde.plasma.icontasks");

const topBar = newPanel();
topBar.location = "top";
topBar.height = Math.ceil(gridUnit * 1.5);
topBar.addWidget("org.kde.plasma.appmenu");
topBar.addWidget("org.kde.plasma.panelspacer");
topBar.addWidget("org.kde.plasma.systemtray");
topBar.addWidget("org.kde.plasma.digitalclock");
