# SPDX-License-Identifier: CC0-1.0
# SPDX-FileCopyrightText: None

[Desktop Entry]
Exec=@CMAKE_INSTALL_FULL_LIBEXECDIR@/plasma-dbus-run-session-if-needed ${CMAKE_INSTALL_FULL_BINDIR}/startplasma-wayland
TryExec=${CMAKE_INSTALL_FULL_BINDIR}/startplasma-wayland
DesktopNames=KDE
Name=Plasma
Comment=Plasma by KDE
X-KDE-PluginInfo-Version=${PROJECT_VERSION}
