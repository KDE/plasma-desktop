# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_nightcolor\")

set(kcm_nightcolor_SRCS
    kcm.cpp
)

add_library(kcm_nightcolor MODULE ${kcm_nightcolor_SRCS})

target_link_libraries(kcm_nightcolor
    KF5::QuickAddons
    KF5::I18n

    PW::LibColorCorrect
)

kcoreaddons_desktop_to_json(kcm_nightcolor "kcm_nightcolor.desktop" SERVICE_TYPES kcmodule.desktop)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES kcm_nightcolor.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(TARGETS kcm_nightcolor DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_nightcolor kcms)
