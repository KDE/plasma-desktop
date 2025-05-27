# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

ecm_add_qml_module(kcm_keyboard_declarative URI "org.kde.plasma.private.kcm_keyboard" SOURCES types.h keyboardmodel.cpp DEPENDENCIES QtCore GENERATE_PLUGIN_SOURCE)

target_link_libraries(kcm_keyboard_declarative PRIVATE kcm_keyboard_static)

ecm_finalize_qml_module(kcm_keyboard_declarative)
