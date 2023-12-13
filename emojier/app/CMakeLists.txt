add_executable(plasma-emojier emojier.cpp)

ecm_add_qml_module(plasma-emojier URI org.kde.plasma.emoji.app)

ecm_target_qml_sources(plasma-emojier SOURCES
    ui/Emojier.qml
    ui/CategoryAction.qml
    ui/CategoryPage.qml
)

target_link_libraries(plasma-emojier PRIVATE Qt::Widgets Qt::Quick KF6::ConfigGui KF6::I18n KF6::CoreAddons KF6::Crash KF6::DBusAddons KF6::WindowSystem Qt::GuiPrivate)

install(TARGETS plasma-emojier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
install(PROGRAMS org.kde.plasma.emojier.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
