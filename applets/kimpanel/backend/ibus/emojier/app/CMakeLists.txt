add_executable(ibus-ui-emojier-plasma emojier.cpp resources.qrc)
target_link_libraries(ibus-ui-emojier-plasma Qt::Widgets Qt::Quick KF5::ConfigGui KF5::I18n KF5::CoreAddons KF5::Crash KF5::DBusAddons KF5::WindowSystem)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(ibus-ui-emojier-plasma Qt::X11Extras)
endif()

install(TARGETS ibus-ui-emojier-plasma ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
install(PROGRAMS org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
