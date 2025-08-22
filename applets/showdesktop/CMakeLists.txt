add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.showdesktop")

qt_add_dbus_interface(showdesktop_SRCS org.kde.KWin.xml kwin_interface)

plasma_add_applet(org.kde.plasma.showdesktop
    QML_SOURCES
        main.qml
        Controller.qml
        MinimizeAllController.qml
        PeekController.qml
    CPP_SOURCES
        showdesktop.cpp
        ${showdesktop_SRCS}
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.showdesktop PRIVATE
     Qt::Core
     Qt::DBus
     Qt::Qml
     Qt::Quick
     KF6::WindowSystem
    )
