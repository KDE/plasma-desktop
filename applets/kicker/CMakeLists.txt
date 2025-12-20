plasma_add_applet(org.kde.plasma.kicker
    QML_SOURCES
    config.qml
    ActionMenu.qml
    CompactRepresentation.qml
    ConfigGeneral.qml
    DashboardRepresentation.qml
    ItemAbstractDelegate.qml
    ItemGridDelegate.qml
    ItemGridView.qml
    ItemListDelegate.qml
    ItemListDialog.qml
    ItemListView.qml
    ItemMultiGridView.qml
    MenuRepresentation.qml
    RunnerResultsList.qml
    SideBarItem.qml
    SideBarSection.qml
    main.qml

    RESOURCES
    main.xml

    GENERATE_APPLET_CLASS
)

ecm_target_qml_sources(org.kde.plasma.kicker SOURCES code/tools.js PATH code)
