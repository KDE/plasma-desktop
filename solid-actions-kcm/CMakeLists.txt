project (solid-actions)

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

qt5_wrap_ui(kcm_solid_actions_srcs
    SolidActions.ui
    AddAction.ui
    ActionEditor.ui )

add_library(kcm_solid_actions MODULE ${kcm_solid_actions_srcs})
add_executable(solid-action-desktop-gen ${solid_action_desktop_gen_srcs})

target_link_libraries(kcm_solid_actions KF5::KIOCore KF5::KIOWidgets KF5::Solid )
target_link_libraries(solid-action-desktop-gen KF5::Solid KF5::KIOCore KF5::KIOWidgets )

########### install files ###############

install( TARGETS kcm_solid_actions DESTINATION ${PLUGIN_INSTALL_DIR} )
install( TARGETS solid-action-desktop-gen ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES solid-actions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES solid-action-template.desktop DESTINATION ${DATA_INSTALL_DIR}/kcmsolidactions )
install( FILES solid-device-type.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

