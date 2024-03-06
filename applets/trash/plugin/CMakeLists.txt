ecm_add_qml_module(trashplugin URI "org.kde.plasma.private.trash" GENERATE_PLUGIN_SOURCE)

target_sources(trashplugin PRIVATE
    dirmodel.cpp
    trash.cpp
    )

target_link_libraries(trashplugin PRIVATE
        Qt::Core
        Qt::Qml
        KF6::KIOCore
        KF6::KIOWidgets
        KF6::CoreAddons # KSharedDataCache required by KImageCache in GuiAddons
        KF6::GuiAddons
        Qt::DBus
        )

ecm_finalize_qml_module(trashplugin)
