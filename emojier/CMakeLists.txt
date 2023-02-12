add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.plasma.emojier\")

kconfig_add_kcfg_files(emojier_KCFG emojiersettings.kcfgc GENERATE_MOC)

add_library(EmojierDeclarativePlugin SHARED emojierplugin.cpp emojidict.cpp emojicategory.cpp ${emojier_KCFG})
target_link_libraries(EmojierDeclarativePlugin PRIVATE Qt::Qml Qt::Gui KF6::ConfigGui KF6::I18n)

install (TARGETS EmojierDeclarativePlugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/emoji)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/emoji)

add_subdirectory(data)
add_subdirectory(app)
