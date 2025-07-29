add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.desktopcontainment\")
add_subdirectory(plugins)

plasma_add_applet(org.kde.desktopcontainment
    QML_SOURCES
        ActionButton.qml
        BackButtonItem.qml
        CompactRepresentation.qml
        ConfigFilter.qml
        ConfigIcons.qml
        ConfigLocation.qml
        ConfigOverlay.qml
        config.qml
        FolderItemActionButton.qml
        FolderItemDelegate.qml
        FolderItemPreviewPluginsDialog.qml
        FolderViewDialog.qml
        FolderViewDropArea.qml
        FolderViewLayer.qml
        FolderView.qml
        main.qml
        RenameEditor.qml
    RESOURCES
        main.xml
    GENERATE_CONTAINMENT_CLASS
)

ecm_target_qml_sources(org.kde.desktopcontainment SOURCES
    code/FolderTools.js
    PATH code
)
