plasma_add_applet(org.kde.plasma.trash
    QML_SOURCES
        main.qml
    CPP_SOURCES
        dirmodel.h
        dirmodel.cpp
        trash.h
        trash.cpp
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.trash PRIVATE
        Qt::Core
        Qt::Qml
        KF6::KIOCore
        KF6::KIOWidgets
        KF6::CoreAddons # KSharedDataCache required by KImageCache in GuiAddons
        KF6::GuiAddons
        KF6::Notifications
        Qt::DBus
        )
