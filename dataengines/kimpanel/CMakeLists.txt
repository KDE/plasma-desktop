add_definitions(-DTRANSLATION_DOMAIN="plasma_engine_kimpanel")

configure_file(config-kimpanel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimpanel.h)

set(plasma_engine_kimpanel_SRCS
    kimpaneldataengine.cpp
    kimpanelagent.cpp
    kimpanelservice.cpp
    kimpaneljob.cpp
    kimpanelinputpanelcontainer.cpp
    kimpanelstatusbarcontainer.cpp
)
QT5_ADD_DBUS_ADAPTOR(plasma_engine_kimpanel_SRCS
    org.kde.impanel.xml
    kimpanelagent.h
    PanelAgent)


add_library(plasma_engine_kimpanel ${plasma_engine_kimpanel_SRCS})
target_include_directories(plasma_engine_kimpanel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(plasma_engine_kimpanel PROPERTIES PREFIX "")
target_link_libraries(plasma_engine_kimpanel
	KF5::Plasma
	KF5::Service
	Qt5::DBus
)
kcoreaddons_desktop_to_json(plasma_engine_kimpanel plasma-dataengine-kimpanel.desktop)

install(TARGETS plasma_engine_kimpanel
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)

install(FILES plasma-dataengine-kimpanel.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

install(FILES kimpanel.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
