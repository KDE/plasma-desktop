# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmformats\")

set(kcm_formats_PART_SRCS
    kcmformats.cpp
)

ki18n_wrap_ui(kcm_formats_PART_SRCS kcmformatswidget.ui )

add_library(kcm_formats MODULE ${kcm_formats_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_formats formats.desktop SERVICE_TYPES kcmodule.desktop)

target_link_libraries(kcm_formats Qt5::Core Qt5::DBus Qt5::Widgets KF5::I18n KF5::ConfigCore KF5::KCMUtils)

install(TARGETS kcm_formats  DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install( FILES formats.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
