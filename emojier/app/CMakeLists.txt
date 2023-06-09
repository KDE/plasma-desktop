add_executable(plasma-emojier emojier.cpp resources.qrc)
target_link_libraries(plasma-emojier Qt::Widgets Qt::Quick KF6::ConfigGui KF6::I18n KF6::CoreAddons KF6::Crash KF6::DBusAddons KF6::WindowSystem)
target_link_libraries(plasma-emojier Qt::GuiPrivate)

install(TARGETS plasma-emojier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
install(PROGRAMS org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
