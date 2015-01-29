# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_standard_actions\")

set(kcm_standard_actions_PART_SRCS standard_actions_module.cpp)

add_library(kcm_standard_actions MODULE ${kcm_standard_actions_PART_SRCS})

target_link_libraries(kcm_standard_actions
    KF5::KCMUtils
    KF5::I18n
    KF5::XmlGui
)

install(TARGETS kcm_standard_actions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES standard_actions.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

