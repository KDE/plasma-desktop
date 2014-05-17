
set(kcm_formats_PART_SRCS
    kcmformats.cpp
)

add_definitions(-DTRANSLATION_DOMAIN=\"kcmformats\")

kservice_desktop_to_json(formats.desktop)

ki18n_wrap_ui(kcm_formats_PART_SRCS kcmformatswidget.ui )

add_library(kcm_formats MODULE ${kcm_formats_PART_SRCS})


target_link_libraries(kcm_formats Qt5::Core Qt5::DBus Qt5::Widgets KF5::I18n KF5::ConfigCore KF5::KCMUtils)

install(TARGETS kcm_formats  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES formats.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

