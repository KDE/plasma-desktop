# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_plasmasearch\")

add_subdirectory(krunner)
add_subdirectory(plasmasearch)
add_subdirectory(plugininstaller)

install(FILES krunner.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
