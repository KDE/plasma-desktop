# SPDX-FileCopyrightText: 2019 Aleix Pol <aleixpol@kde.org>
# SPDX-FileCopyrightText: 2025 Neal Gompa <ngompa@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

configure_file(plasmawayland.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/plasma.desktop)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/plasma.desktop
        DESTINATION ${KDE_INSTALL_DATADIR}/wayland-sessions
)
