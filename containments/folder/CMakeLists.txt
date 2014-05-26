plasma_install_package(package org.kde.plasma.folder)

set(folderplugin_SRCS
    plugin/directorypicker.cpp
    plugin/foldermodel.cpp
    plugin/folderplugin.cpp
    plugin/itemviewadapter.cpp
    plugin/labelgenerator.cpp
    plugin/mimetypesmodel.cpp
    plugin/placesmodel.cpp
    plugin/previewpluginsmodel.cpp
    plugin/systemsettings.cpp
    plugin/internallibkonq/konq_copytomenu.cpp
    plugin/internallibkonq/konq_dndpopupmenuplugin.cpp
    plugin/internallibkonq/konq_fileitemcapabilities.cpp
    plugin/internallibkonq/konqmimedata.cpp
    plugin/internallibkonq/konq_popupmenu.cpp
    plugin/internallibkonq/konq_popupmenuinformation.cpp
    plugin/internallibkonq/konq_popupmenuplugin.cpp
    plugin/internallibkonq/konq_operations.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/folder)

add_library(folderplugin SHARED ${folderplugin_SRCS})

target_link_libraries(folderplugin
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Widgets
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::KIOFileWidgets
                      KF5::KDELibs4Support)

install(TARGETS folderplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/folder)
