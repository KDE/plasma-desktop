add_definitions(-DKSMSERVER_UNIT_TEST)
include(ECMMarkAsTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)
set( kcmTest_SRCS
     kcmtest.cpp
     ../kcm.cpp
     ../../krdb/krdb.cpp
     ../../input/xcursor/cursortheme.cpp
     ../../input/xcursor/xcursortheme.cpp
)

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcmTest_SRCS ${klauncher_xml} klauncher_iface)

add_executable(kcmTest ${kcmTest_SRCS})

target_link_libraries(kcmTest
        Qt5::Test
        KF5::KIOWidgets
        KF5::CoreAddons
        KF5::KCMUtils
        KF5::I18n
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::KDELibs4Support

        Qt5::DBus
        Qt5::Widgets
        Qt5::QuickWidgets
        ${X11_LIBRARIES}
)

if(X11_FOUND)
    target_link_libraries(kcmTest Qt5::X11Extras)
endif()
if (X11_Xcursor_FOUND)
   target_link_libraries(kcmTest ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcmTest ${X11_Xfixes_LIB})
endif ()

add_test(lookandfeel-kcmTest kcmTest)
ecm_mark_as_test(kcmTest)

