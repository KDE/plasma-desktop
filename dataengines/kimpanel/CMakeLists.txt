project(plasma-dataengine-kimpanel)

set(plasma_engine_kimpanel_SRCS
    kimpaneldataengine.cpp
    kimpanelagent.cpp
    kimpanelservice.cpp
    kimpaneljob.cpp
    kimpanelinputpanelcontainer.cpp
    kimpanelstatusbarcontainer.cpp
)
QT4_ADD_DBUS_ADAPTOR(plasma_engine_kimpanel_SRCS
    org.kde.impanel.xml
    kimpanelagent.h
    PanelAgent)

kde4_add_plugin(plasma_engine_kimpanel ${plasma_engine_kimpanel_SRCS})
target_link_libraries(plasma_engine_kimpanel
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_kimpanel
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-dataengine-kimpanel.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES kimpanel.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
