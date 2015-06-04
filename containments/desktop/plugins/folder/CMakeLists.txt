include_directories(plasmaquick)

set(folderplugin_SRCS
    directorypicker.cpp
    foldermodel.cpp
    folderplugin.cpp
    itemviewadapter.cpp
    labelgenerator.cpp
    menuhelper.cpp
    mimetypesmodel.cpp
    placesmodel.cpp
    positioner.cpp
    previewpluginsmodel.cpp
    rubberband.cpp
    subdialog.cpp
    textfix.cpp
    viewpropertiesmenu.cpp
    wheelinterceptor.cpp
    internallibkonq/konq_copytomenu.cpp
    internallibkonq/konq_dndpopupmenuplugin.cpp
    internallibkonq/konq_popupmenu.cpp
    internallibkonq/konq_popupmenuplugin.cpp
)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/folder)

add_library(folderplugin SHARED ${folderplugin_SRCS})

target_link_libraries(folderplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::KIOFileWidgets
                      KF5::KDELibs4Support
                      KF5::PlasmaQuick)

install(TARGETS folderplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/private/desktopcontainment/folder)
