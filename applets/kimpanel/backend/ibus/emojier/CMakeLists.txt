kconfig_add_kcfg_files(emojier_KCFG emojiersettings.kcfgc GENERATE_MOC)

add_library(EmojierDeclarativePlugin SHARED emojierplugin.cpp ${emojier_KCFG})
target_link_libraries(EmojierDeclarativePlugin PRIVATE GLIB2::GLIB2 PkgConfig::IBUS Qt5::Qml Qt5::Gui KF5::ConfigGui)

install (TARGETS EmojierDeclarativePlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/emoji)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/emoji)

add_subdirectory(app)
