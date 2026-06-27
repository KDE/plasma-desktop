// SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

// XFCE-like: a top panel with menu, window list and status area, plus a small launcher dock at the bottom.
clearPanels();

const topPanel = newPanel();
topPanel.location = "top";
topPanel.height = Math.ceil(gridUnit * 1.6);
topPanel.addWidget("org.kde.plasma.kickoff");
topPanel.addWidget("org.kde.plasma.taskmanager");
topPanel.addWidget("org.kde.plasma.marginsseparator");
topPanel.addWidget("org.kde.plasma.systemtray");
topPanel.addWidget("org.kde.plasma.digitalclock");
topPanel.addWidget("org.kde.plasma.showdesktop");

const dock = newPanel();
dock.location = "bottom";
dock.height = Math.ceil(gridUnit * 2.5);
dock.alignment = "center";
dock.lengthMode = "fit";
dock.floating = true;
dock.addWidget("org.kde.plasma.icontasks");
