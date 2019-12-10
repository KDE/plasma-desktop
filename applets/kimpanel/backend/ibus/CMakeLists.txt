find_package(IBus 1.5.0)
find_package(GLIB2)
find_package(GIO)
find_package(GObject)

if(IBUS_FOUND AND GLIB2_FOUND AND GIO_FOUND AND GOBJECT_FOUND)
    include_directories(${IBUS_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR})
    add_subdirectory(emojier)

    configure_file(config-kimpanel.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kimpanel.h)
    find_package(Qt5X11Extras)
    find_package(XCB COMPONENTS XCB KEYSYMS)

    if (Qt5X11Extras_FOUND AND XCB_XCB_FOUND AND XCB_KEYSYMS_FOUND)
        include_directories(${GIO_INCLUDE_DIR})
        include_directories(${GOBJECT_INCLUDE_DIR})
        include_directories(${XCB_XCB_INCLUDE_DIRS})
        include_directories(${XCB_KEYSYMS_INCLUDE_DIRS})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
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
        target_link_libraries(kimpanel-ibus-panel ${IBUS_LIBRARIES} GLIB2::GLIB2 ${GIO_LIBRARIES} ${GOBJECT_LIBRARIES} Qt5::Core Qt5::DBus Qt5::Gui Qt5::X11Extras XCB::KEYSYMS)
        # configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kimpanel.xml.in ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml @ONLY)
        # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component)

        target_include_directories(kimpanel-ibus-panel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

        add_executable(kimpanel-ibus-panel-launcher launcher.cpp)
        target_link_libraries(kimpanel-ibus-panel-launcher Qt5::Core Qt5::DBus)
        install(TARGETS kimpanel-ibus-panel kimpanel-ibus-panel-launcher DESTINATION ${KDE_INSTALL_LIBEXECDIR})
    endif()
endif()

