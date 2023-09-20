add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.plasma.emojier\")

ecm_add_qml_module(EmojierDeclarativePlugin URI org.kde.plasma.emoji GENERATE_PLUGIN_SOURCE)
target_sources(EmojierDeclarativePlugin PRIVATE
    emojierplugin.cpp
    emojidict.cpp
    emojicategory.cpp
)

kconfig_add_kcfg_files(EmojierDeclarativePlugin emojiersettings.kcfgc GENERATE_MOC)

target_link_libraries(EmojierDeclarativePlugin PRIVATE Qt::Qml Qt::Gui KF6::ConfigGui KF6::I18n)

ecm_finalize_qml_module(EmojierDeclarativePlugin)

add_subdirectory(data)
add_subdirectory(app)
