# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

add_library(kcm_keyboard_declarative SHARED plugin.cpp)

target_link_libraries(kcm_keyboard_declarative PRIVATE kcm_keyboard_static)

install(TARGETS kcm_keyboard_declarative DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kcm_keyboard)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kcm_keyboard)
