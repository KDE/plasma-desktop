if(IBUS_FOUND AND GLIB2_FOUND AND GOBJECT_FOUND)
    add_subdirectory(emojier)

    configure_file(config-kimpanel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimpanel.h)
    find_package(Qt5X11Extras)
    find_package(XCB COMPONENTS XCB KEYSYMS)

    if (Qt5X11Extras_FOUND AND XCB_XCB_FOUND AND XCB_KEYSYMS_FOUND)
        set(CMAKE_C_STANDARD 99)
        add_executable(kimpanel-ibus-panel
            ibus15/xkblayoutmanager.cpp
            ibus15/gtkaccelparse_p.c
            ibus15/gdkkeynames_p.c
            ibus15/app.cpp
            ibus15/enginemanager.cpp
            ibus15/main.cpp
            ibus15/panel.cpp
            ibus15/propertymanager.cpp)
        target_compile_definitions(kimpanel-ibus-panel PRIVATE -DQT_NO_KEYWORDS)
        target_link_libraries(kimpanel-ibus-panel GLIB2::GLIB2 PkgConfig::IBUS PkgConfig::GOBJECT Qt::Core Qt::DBus Qt::Gui Qt::X11Extras XCB::KEYSYMS)
        # configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kimpanel.xml.in ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml @ONLY)
        # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component)

        target_include_directories(kimpanel-ibus-panel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

        add_executable(kimpanel-ibus-panel-launcher launcher.cpp)
        target_link_libraries(kimpanel-ibus-panel-launcher Qt::Core Qt::DBus)
        install(TARGETS kimpanel-ibus-panel kimpanel-ibus-panel-launcher DESTINATION ${KDE_INSTALL_LIBEXECDIR})
    endif()
endif()

