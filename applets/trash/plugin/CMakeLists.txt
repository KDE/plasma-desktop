set(trashplugin_SRCS
    dirmodel.cpp
    trash.cpp
    trashplugin.cpp
    )

add_library(trashplugin SHARED ${trashplugin_SRCS})
target_link_libraries(trashplugin
        Qt::Core
        Qt::Qml
        KF5::KIOCore
        KF5::KIOWidgets
        KF5::CoreAddons # KSharedDataCache required by KImageCache in GuiAddons
        KF5::GuiAddons
        Qt::DBus
        )

install(TARGETS trashplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/trash)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/trash)
