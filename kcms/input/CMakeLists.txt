if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "Xinput not found")
endif()

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcminput\")

add_subdirectory( pics )

## Add common files here.
set(kcminput_backend_SRCS
    mousebackend.cpp
    mousesettings.cpp
    logging.cpp)
set(kcminput_backend_LIBS)
include(backends/x11.cmake)


########### next target ###############

set(kcm_input_PART_SRCS
    mouse.cpp
    main.cpp
    ${kcminput_backend_SRCS}
)

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)

ki18n_wrap_ui(kcm_input_PART_SRCS kcmmouse.ui)
qt5_add_dbus_interface(kcm_input_PART_SRCS ${klauncher_xml} klauncher_iface)

add_library(kcm_input MODULE ${kcm_input_PART_SRCS} ${kcminput_backend_SRCS})

target_link_libraries(kcm_input
    Qt5::DBus
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDELibs4Support
    ${kcminput_backend_LIBS}
)

install(TARGETS kcm_input  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mouse.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz  DESTINATION  ${DATA_INSTALL_DIR}/kcminput )

