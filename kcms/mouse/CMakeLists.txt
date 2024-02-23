# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmmouse\")

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
endif()

add_subdirectory(misc)

add_library(mouse_common STATIC inputbackend.cpp)

ecm_qt_declare_logging_category(mouse_common
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

set_property(TARGET mouse_common PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(mouse_common
    PUBLIC
        KF6::ConfigCore
        KF6::I18n
        KF6::WindowSystem
        PW::KWorkspace
)

add_subdirectory(backends)

if (BUILD_KCM_MOUSE_X11)
    # kapplymousetheme is an X11-only tool, called by startplasma-x11 but not startplasma-wayland
    add_executable(kapplymousetheme kapplymousetheme.cpp)
    target_link_libraries(kapplymousetheme Qt::Gui KF6::CoreAddons mouse_common)
    install(TARGETS kapplymousetheme ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

add_library(kcm_mouse MODULE
    plugin.cpp
    configcontainer.cpp
    ui/libinput_config.cpp
)
qt_add_resources(kcm_mouse
    PREFIX "/"
    FILES
        ui/main.qml
        ui/main_deviceless.qml
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
