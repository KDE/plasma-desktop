# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmmouse\")

add_subdirectory( misc )

## Add common files here.
set(common_SRCS
    inputbackend.cpp
)

ecm_qt_declare_logging_category(common_SRCS
    HEADER
        logging.h
    IDENTIFIER
        KCM_MOUSE
    CATEGORY_NAME
        kcm_mouse
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "KCM for mouse input"
    EXPORT
        kcm_mouse
)
ecm_qt_install_logging_categories(
    EXPORT kcm_mouse
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

if (BUILD_KCM_MOUSE_KWIN_WAYLAND)
    include(backends/kwin_wl.cmake)
endif()

if (BUILD_KCM_MOUSE_X11)
    if (NOT EVDEV_FOUND)
        list(APPEND MISSING_DEPS "xorg-evdev")
    endif()
    if (NOT XORGLIBINPUT_FOUND)
        list(APPEND MISSING_DEPS "xorg-libinput")
    endif()
    if (NOT X11_Xi_FOUND)
        list(APPEND MISSING_DEPS "X11_Xi")
    endif()
    if (NOT X11_Xinput_FOUND)
        list(APPEND MISSING_DEPS "X11_Xinput")
    endif()
    if (MISSING_DEPS)
        message(FATAL_ERROR "Missing X11 dependencies for kcm_mouse: ${MISSING_DEPS}. Install dependencies or set CMake option -DBUILD_KCM_MOUSE_X11=OFF.")
    endif()

    include(backends/x11.cmake)

    # kapplymousetheme is an X11-only tool, called by startplasma-x11 but not startplasma-wayland
    add_executable(kapplymousetheme
        kapplymousetheme.cpp
        ${common_SRCS}
        ${backend_SRCS}
    )
    target_link_libraries(kapplymousetheme
        ${backend_LIBS}
        Qt::Gui
        Qt::DBus
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::I18n
        KF6::WindowSystem
        PW::KWorkspace
    )
    install(TARGETS kapplymousetheme ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

    ########### next target ###############

    list(APPEND common_SRCS
        kcm/xlib/xlib_config.cpp
    )
    ki18n_wrap_ui(common_SRCS kcm/xlib/kcmmouse.ui)
endif()

qt_add_resources( common_SRCS kcm/resources.qrc )

add_library(kcm_mouse MODULE
    ${common_SRCS}
    ${backend_SRCS}
    plugin.cpp
    kcm/configcontainer.cpp
    kcm/configplugin.cpp
    kcm/libinput/libinput_config.cpp
)
kcmutils_generate_desktop_file(kcm_mouse)

target_link_libraries(kcm_mouse
    ${backend_LIBS}
    KF6::KCMUtils
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WindowSystem
    PW::KWorkspace
    Qt::DBus
    Qt::QuickWidgets
)

install(TARGETS kcm_mouse DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)

########### install files ###############


add_custom_command(TARGET kcm_mouse POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_mouse.so" "kcm_mouse_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_mouse_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)
