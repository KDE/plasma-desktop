add_library(kcm_mouse MODULE
    plugin.cpp
    configcontainer.cpp
    configplugin.cpp
    libinput/libinput_config.cpp
)
qt_add_resources(kcm_mouse
    PREFIX "/"
    FILES
        libinput/main.qml
)
kcmutils_generate_desktop_file(kcm_mouse)

target_link_libraries(kcm_mouse
    KF6::KCMUtils
    KF6::KIOCore
    KF6::KIOWidgets
    Qt::QuickWidgets
    mouse_common
)

install(TARGETS kcm_mouse DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)

########### install files ###############

add_custom_command(TARGET kcm_mouse POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_mouse.so" "kcm_mouse_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_mouse_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)
