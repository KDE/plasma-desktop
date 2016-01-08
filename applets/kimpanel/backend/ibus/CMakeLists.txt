find_package(IBus)
find_package(GLIB2)
find_package(GIO)
find_package(GObject)
if(IBUS_FOUND AND GLIB2_FOUND AND GIO_FOUND AND GOBJECT_FOUND)
    include_directories(${IBUS_INCLUDE_DIR})
    include_directories(${GLIB2_INCLUDE_DIR})
    include_directories(${GIO_INCLUDE_DIR})
    include_directories(${GOBJECT_INCLUDE_DIR})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
    if (${IBUS_VERSION} VERSION_LESS "1.5.0")
        set(kimpanel_ibus_panel_SRCS ibus14/panel.c ibus14/main.c)
        add_executable(kimpanel-ibus-panel ${kimpanel_ibus_panel_SRCS})
        target_link_libraries(kimpanel-ibus-panel ${IBUS_LIBRARIES} ${GLIB2_LIBRARIES} ${GIO_LIBRARIES} ${GOBJECT_LIBRARIES})
        install(TARGETS kimpanel-ibus-panel DESTINATION ${LIBEXEC_INSTALL_DIR})
        # configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kimpanel.xml.in ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml @ONLY)
        # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component)
    else()
        find_package(Qt5X11Extras)
        find_package(XCB COMPONENTS XCB KEYSYMS)
        if (Qt5X11Extras_FOUND AND XCB_XCB_FOUND AND XCB_KEYSYMS_FOUND)
            include_directories(${Qt5X11Extras_INCLUDE_DIRS})
            include_directories(${XCB_XCB_INCLUDE_DIRS})
            include_directories(${XCB_KEYSYMS_INCLUDE_DIRS})
            set(kimpanel_ibus_panel_SRCS
                ibus15/xkblayoutmanager.cpp
                ibus15/gtkaccelparse_p.c
                ibus15/gdkkeynames_p.c
                ibus15/app.cpp
                ibus15/enginemanager.cpp
                ibus15/main.cpp
                ibus15/panel.cpp
                ibus15/propertymanager.cpp)
            add_definitions(-DQT_NO_KEYWORDS)
            add_executable(kimpanel-ibus-panel ${kimpanel_ibus_panel_SRCS})
            target_link_libraries(kimpanel-ibus-panel ${IBUS_LIBRARIES} ${GLIB2_LIBRARIES} ${GIO_LIBRARIES} ${GOBJECT_LIBRARIES}  Qt5::Core Qt5::DBus Qt5::Gui Qt5::X11Extras XCB::KEYSYMS)
            install(TARGETS kimpanel-ibus-panel DESTINATION ${LIBEXEC_INSTALL_DIR})
            # configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kimpanel.xml.in ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml @ONLY)
            # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component)
        endif()
    endif()
endif()

