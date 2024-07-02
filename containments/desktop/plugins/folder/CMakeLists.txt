include(GenerateExportHeader)

if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif(BUILD_TESTING)

set(folderplugin_SRCS
    applauncher.cpp
    desktopschemehelper.cpp
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
    eventgenerator.cpp
)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/folder)

add_library(folderplugin SHARED ${folderplugin_SRCS})

target_link_libraries(folderplugin
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      Plasma::Activities
                      KF6::CoreAddons
                      KF6::KIOCore
                      KF6::KIOWidgets
                      KF6::KIOFileWidgets
                      KF6::I18n
                      Plasma::PlasmaQuick
                      KF6::ConfigGui
                      KF6::Notifications
                      KF6::XmlGui)

install(TARGETS folderplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/desktopcontainment/folder)

ecm_qt_declare_logging_category(folderplugin
    HEADER debug.h
    IDENTIFIER FOLDER
    CATEGORY_NAME org.kde.plasma.folder
)

generate_export_header(folderplugin BASE_NAME folderplugin)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif(BUILD_TESTING)
