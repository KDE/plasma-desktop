include(GenerateExportHeader)

if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif(BUILD_TESTING)

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
    viewpropertiesmenu.cpp
    wheelinterceptor.cpp
    shortcut.cpp
    screenmapper.cpp
    removeaction.cpp
)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/folder)

add_library(folderplugin SHARED ${folderplugin_SRCS})

target_link_libraries(folderplugin
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      KF5::CoreAddons
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::KIOFileWidgets
                      KF5::I18n
                      KF5::PlasmaQuick
                      KF5::ConfigGui)

install(TARGETS folderplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/folder)

generate_export_header(folderplugin BASE_NAME folderplugin)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif(BUILD_TESTING)
