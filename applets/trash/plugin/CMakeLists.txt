project(trashplugin)

set(trashplugin_SRCS
    dirmodel.cpp
    trashplugin.cpp
    )


add_library(trashplugin SHARED ${trashplugin_SRCS})
target_link_libraries(trashplugin
        Qt5::Core
        Qt5::Qml
        KF5::KIOCore
        KF5::KIOWidgets
        KF5::GuiAddons
        )

install(TARGETS trashplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/trash)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/trash)

