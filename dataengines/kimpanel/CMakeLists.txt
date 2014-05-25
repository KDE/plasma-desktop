include_directories("${PROJECT_SOURCE_DIR}/libs")

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

kservice_desktop_to_json(plasma-dataengine-kimpanel.desktop)

add_library(plasma_engine_kimpanel ${plasma_engine_kimpanel_SRCS})
set_target_properties(plasma_engine_kimpanel PROPERTIES PREFIX "")
target_link_libraries(plasma_engine_kimpanel
	KF5::Plasma
	KF5::Service
	Qt5::DBus
)

install(TARGETS plasma_engine_kimpanel
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

install(FILES plasma-dataengine-kimpanel.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kimpanel.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
