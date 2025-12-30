add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.taskmanager\")

plasma_add_applet(org.kde.plasma.taskmanager
    QML_SOURCES
        qml/AudioStream.qml
        qml/Badge.qml
        qml/ConfigAppearance.qml
        qml/ConfigBehavior.qml
        qml/config.qml
        qml/ContextMenu.qml
        qml/GroupDialog.qml
        qml/GroupExpanderOverlay.qml
        qml/main.qml
        qml/MouseHandler.qml
        qml/PipeWireThumbnail.qml
        qml/PlayerController.qml
        qml/ScrollableTextWrapper.qml
        qml/TaskBadgeOverlay.qml
        qml/TaskList.qml
        qml/TaskProgressOverlay.qml
        qml/Task.qml
        qml/ToolTipDelegate.qml
        qml/ToolTipInstance.qml
        qml/ToolTipWindowMouseArea.qml
        qml/code/LayoutMetrics.js
        qml/code/TaskTools.js
    CPP_SOURCES
        backend.cpp
        smartlauncherbackend.cpp
        smartlauncheritem.cpp
        audioinfo.cpp
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

kconfig_add_kcfg_files(org.kde.plasma.taskmanager ${CMAKE_SOURCE_DIR}/kcms/recentFiles/kactivitymanagerd_plugins_settings.kcfgc)

ecm_qt_declare_logging_category(org.kde.plasma.taskmanager
    HEADER log_settings.h
    IDENTIFIER TASKMANAGER_DEBUG
    CATEGORY_NAME org.kde.plasma.taskmanager)

# FIXME Cleanup no longer used libs.
target_link_libraries(org.kde.plasma.taskmanager PRIVATE
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      Plasma::Activities
                      Plasma::ActivitiesStats
                      KF6::ConfigGui
                      KF6::I18n
                      KF6::KIOCore
                      KF6::KIOGui
                      KF6::KIOFileWidgets # KFilePlacesModel
                      KF6::Notifications # KNotificationJobUiDelegate
                      Plasma::Plasma
                      KSysGuard::ProcessCore
                      KF6::Service
                      KF6::WindowSystem
                      KF6::PulseAudioQt
                      PW::LibNotificationManager)
