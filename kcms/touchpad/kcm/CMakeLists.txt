set(KCM_TOUCHPAD_SRCS
    touchpadconfigcontainer.cpp
    touchpadconfigplugin.cpp
    libinput/touchpadconfiglibinput.cpp
    resources.qrc
)

add_library(kcm_touchpad MODULE ${KCM_TOUCHPAD_SRCS})
kcmutils_generate_desktop_file(kcm_touchpad)

target_link_libraries(kcm_touchpad
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    Qt::QuickWidgets
    Qt::DBus
    KF6::CoreAddons
    KF6::I18n
    KF6::Completion
    KF6::KCMUtils
    KF6::Plasma
    KF6::WindowSystem
    KF6::NotifyConfig
    KF6::XmlGui
    touchpad
)

install(TARGETS kcm_touchpad
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings
)

add_custom_command(TARGET kcm_touchpad POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_touchpad.so" "kcm_touchpad_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_touchpad_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)
