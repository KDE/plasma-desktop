add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.desktopcontainment\")
add_subdirectory(plugins)
plasma_install_package(package org.kde.desktopcontainment plasmoids containment)
