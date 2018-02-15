add_definitions(-DTRANSLATION_DOMAIN="kcm_solid_actions")

add_subdirectory(device-actions)

########### next target ###############

set(kcm_solid_actions_srcs
    PredicateItem.cpp
    PredicateModel.cpp
    ActionItem.cpp
    ActionModel.cpp
    ActionEditor.cpp
    SolidActionData.cpp
    SolidActions.cpp )

set(solid_action_desktop_gen_srcs
    DesktopFileGenerator.cpp
    SolidActionData.cpp )

ki18n_wrap_ui(kcm_solid_actions_srcs
    SolidActions.ui
    AddAction.ui
    ActionEditor.ui )

add_library(kcm_solid_actions MODULE ${kcm_solid_actions_srcs})
add_executable(solid-action-desktop-gen ${solid_action_desktop_gen_srcs})

target_link_libraries(kcm_solid_actions KF5::KIOCore KF5::KIOWidgets KF5::Solid KF5::I18n KF5::IconThemes KF5::ConfigWidgets KF5::CoreAddons)
target_link_libraries(solid-action-desktop-gen KF5::Solid KF5::KIOCore KF5::I18n KF5::KIOWidgets)

########### install files ###############

install( TARGETS kcm_solid_actions DESTINATION ${KDE_INSTALL_PLUGINDIR} )
install( TARGETS solid-action-desktop-gen ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES solid-actions.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES solid-action-template.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kcmsolidactions )
install( FILES solid-device-type.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR} )
