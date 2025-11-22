if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif(BUILD_TESTING)

ecm_add_qml_module(folderplugin URI org.kde.private.desktopcontainment.folder GENERATE_PLUGIN_SOURCE DEPENDENCIES "QtCore" "QtQuick")

target_sources(folderplugin PRIVATE
    applauncher.cpp
    desktopschemehelper.cpp
    directorypicker.cpp
    foldermodel.cpp
    itemviewadapter.cpp
    labelgenerator.cpp
    menuhelper.cpp
    mimetypesmodel.cpp
    placesmodel.cpp
    positioner.cpp
    previewpluginsmodel.cpp
    subdialog.cpp
    viewpropertiesmenu.cpp
    wheelinterceptor.cpp
    shortcut.cpp
    screenmapper.cpp
    removeaction.cpp
    eventgenerator.cpp
    types.h
)

target_link_libraries(folderplugin PUBLIC
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

ecm_qt_declare_logging_category(folderplugin
    HEADER debug.h
    IDENTIFIER FOLDER
    CATEGORY_NAME org.kde.plasma.folder
)

ecm_finalize_qml_module(folderplugin)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif(BUILD_TESTING)
