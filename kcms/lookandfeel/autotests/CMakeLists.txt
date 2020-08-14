add_definitions(-DKSMSERVER_UNIT_TEST)
include(ECMMarkAsTest)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)
set( kcmTest_SRCS
     kcmtest.cpp
     ../kcm.cpp
     ../lookandfeeldata.cpp
     ../../krdb/krdb.cpp
)

if (X11_Xcursor_FOUND)
    set(kcmTest_SRCS
        ${kcmTest_SRCS}
        ../../cursortheme/xcursor/cursortheme.cpp
        ../../cursortheme/xcursor/xcursortheme.cpp
    )
endif ()

kconfig_add_kcfg_files(kcmTest_SRCS ../lookandfeelsettings.kcfgc GENERATE_MOC)

add_executable(kcmTest ${kcmTest_SRCS})

add_dependencies(kcmTest kcm_lookandfeel)

target_link_libraries(kcmTest
        Qt5::Test
        KF5::KIOWidgets
        KF5::CoreAddons
        KF5::KCMUtils
        KF5::I18n
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::KDELibs4Support
        KF5::Declarative
        KF5::QuickAddons
        KF5::NewStuff
        PW::KWorkspace

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

add_test(NAME lookandfeel-kcmTest COMMAND kcmTest)
ecm_mark_as_test(kcmTest)

