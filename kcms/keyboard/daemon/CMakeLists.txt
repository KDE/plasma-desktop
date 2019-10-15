
### kded daemon ###

option(HAVE_XINPUT "X11 XInput" OFF)
option(NEW_GEOMETRY "Keyboard geometry preview" OFF)

if (X11_Xinput_FOUND)
    set(HAVE_XINPUT ON)
else()
    message(STATUS "X11 XInput is not found, either is required to keep layouts with keyboard hotplugging")
endif()

set(kded_keyboard_SRCS
    keyboard_daemon.cpp
    x11_helper.cpp
    xinput_helper.cpp
    xkb_helper.cpp
    keyboard_hardware.cpp
    bindings.cpp
)

if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
    set(logging_severity Debug)
else()
    set(logging_severity Warning)
endif()

ecm_qt_declare_logging_category(kded_keyboard_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY ${logging_severity})


add_library(kded_keyboard MODULE ${kded_keyboard_SRCS})
set_target_properties(kded_keyboard PROPERTIES OUTPUT_NAME keyboard)
kcoreaddons_desktop_to_json(kded_keyboard keyboard.desktop SERVICE_TYPES kdedmodule.desktop)

target_include_directories(kded_keyboard PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(kded_keyboard
    Qt5::Concurrent
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::Service
    KF5::WindowSystem
    KF5::Plasma
    KF5::XmlGui
    KF5::GuiAddons
    KF5::IconThemes

    kcm_keyboard_layoutmodel
)

if (HAVE_XINPUT)
    target_link_libraries(kded_keyboard ${X11_Xinput_LIB})
endif()

if (HAVE_UDEV)
    target_link_libraries(kded_keyboard ${UDEV_LIBS})
endif()

install(TARGETS kded_keyboard DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

configure_file(../config-keyboard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keyboard.h)
