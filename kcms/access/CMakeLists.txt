# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmaccess\")

set(kcm_access_PART_SRCS kcmaccess.cpp )

ki18n_wrap_ui(kcm_access_PART_SRCS
  accessibility.ui
)

add_library(kcm_access MODULE ${kcm_access_PART_SRCS})

target_link_libraries(kcm_access
    Qt5::X11Extras
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::NotifyConfig
    KF5::WindowSystem
    ${X11_LIBRARIES}
)

install(TARGETS kcm_access DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### install files ###############

install( FILES kcmaccess.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
