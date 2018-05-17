# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_workspace\")

########### next target ###############
set(kcm_workspace_SRCS
  workspaceoptions.cpp
)

add_library(kcm_workspace MODULE ${kcm_workspace_SRCS})

target_link_libraries(kcm_workspace
    KF5::QuickAddons
    KF5::I18n
    KF5::ConfigWidgets
    KF5::Declarative
    KF5::KDELibs4Support

    Qt5::DBus
)

kcoreaddons_desktop_to_json(kcm_workspace "kcm_workspace.desktop" SERVICE_TYPES kcmodule.desktop)

########### install files ###############
install( FILES kcm_workspace.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm_workspace DESTINATION ${KDE_INSTALL_PLUGINDIR}/kcms)

kpackage_install_package(package kcm_workspace kcms)
