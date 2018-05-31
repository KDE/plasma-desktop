include(ECMQMLModules)
ecm_find_qmlmodule(org.kde.plasma.core 2.0)

# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_translations\")

########### next target ###############

set(kcm_translations_PART_SRCS translations.cpp translationsmodel.cpp)

add_library(kcm_translations MODULE ${kcm_translations_PART_SRCS})

target_link_libraries(kcm_translations
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

kcoreaddons_desktop_to_json(kcm_translations "kcm_translations.desktop")

########### install files ###############

install(TARGETS kcm_translations DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)
install(FILES kcm_translations.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
kpackage_install_package(package kcm_translations kcms)

