// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// GNOME 3-like: a single top bar with an Activities-style launcher on the left, a centered clock
// and a status area on the right. Apps are launched and windows switched through the full-screen
// Application Dashboard.
clearPanels();

const topBar = newPanel();
topBar.location = "top";
topBar.height = Math.ceil(gridUnit * 1.6);

topBar.addWidget("org.kde.plasma.kickerdash");
topBar.addWidget("org.kde.plasma.panelspacer");
topBar.addWidget("org.kde.plasma.digitalclock");
topBar.addWidget("org.kde.plasma.panelspacer");
topBar.addWidget("org.kde.plasma.systemtray");
