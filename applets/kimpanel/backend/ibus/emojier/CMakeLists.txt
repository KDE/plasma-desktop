kconfig_add_kcfg_files(emojier_KCFG emojiersettings.kcfgc GENERATE_MOC)

add_executable(ibus-ui-emojier-plasma emojier.cpp resources.qrc ${emojier_KCFG})
target_link_libraries(ibus-ui-emojier-plasma Qt5::Widgets ${IBUS_LIBRARIES} ${GOBJECT_LIBRARIES} Qt5::Quick KF5::ConfigGui KF5::I18n KF5::CoreAddons KF5::Crash KF5::QuickAddons KF5::DBusAddons)

install(TARGETS ibus-ui-emojier-plasma ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.emojier.desktop DESTINATION ${DATA_INSTALL_DIR}/kglobalaccel)
install(PROGRAMS org.kde.plasma.emojier.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
