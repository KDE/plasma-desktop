// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// macOS-like: a thin global menu bar along the top and a floating, centered dock at the bottom.
clearPanels();

const menuBar = newPanel();
menuBar.location = "top";
menuBar.height = Math.ceil(gridUnit * 1.5);
menuBar.addWidget("org.kde.plasma.kickoff");
menuBar.addWidget("org.kde.plasma.appmenu");
menuBar.addWidget("org.kde.plasma.panelspacer");
menuBar.addWidget("org.kde.plasma.systemtray");
menuBar.addWidget("org.kde.plasma.digitalclock");

const dock = newPanel();
dock.location = "bottom";
dock.height = gridUnit * 3;
dock.alignment = "center";
dock.lengthMode = "fit";
dock.floating = true;
dock.addWidget("org.kde.plasma.icontasks");
dock.addWidget("org.kde.plasma.trash");
