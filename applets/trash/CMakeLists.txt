add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.trash\")

add_subdirectory(plugin)
plasma_install_bundled_package(package org.kde.plasma.trash)
