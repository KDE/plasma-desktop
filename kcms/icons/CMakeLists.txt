add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_icons\")

########### next target ###############

set(kcm_icons_PART_SRCS main.cpp iconsmodel.cpp iconsizecategorymodel.cpp iconssettings.cpp)

kconfig_add_kcfg_files(kcm_icons_PART_SRCS iconssettingsbase.kcfgc GENERATE_MOC)
add_library(kcm_icons MODULE ${kcm_icons_PART_SRCS})

target_link_libraries(kcm_icons
    Qt5::Widgets
    Qt5::Svg
    KF5::KCMUtils
    KF5::I18n
    KF5::IconThemes
    KF5::Archive
    KF5::KIOWidgets
    KF5::QuickAddons
)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.h CONTENT "#define CMAKE_INSTALL_FULL_LIBEXECDIR \"${CMAKE_INSTALL_FULL_LIBEXECDIR}\"")

set(changeicons_SRCS changeicons.cpp iconssettings.cpp)

kconfig_add_kcfg_files(changeicons_SRCS iconssettingsbase.kcfgc GENERATE_MOC)

add_executable(plasma-changeicons ${changeicons_SRCS})
target_link_libraries(plasma-changeicons PRIVATE Qt5::Core KF5::KIOWidgets KF5::KCMUtils KF5::IconThemes)

kcoreaddons_desktop_to_json(kcm_icons "kcm_icons.desktop")

install(FILES iconssettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES icons_remove_effects.upd DESTINATION ${KDE_INSTALL_DATADIR}/kconf_update)
install(FILES kcm_icons.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_icons DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/icons.knsrc ${CMAKE_BINARY_DIR}/icons.knsrc)
install( FILES ${CMAKE_BINARY_DIR}/icons.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR} )

install(TARGETS plasma-changeicons DESTINATION ${KDE_INSTALL_LIBEXECDIR} )

kpackage_install_package(package kcm5_icons kcms)
