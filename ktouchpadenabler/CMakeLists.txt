if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "The X11 input library is needed for ktouchpadenabler")
endif()

if (X11_Xinput_FOUND)
    include_directories (${X11_INCLUDE_DIR})

    set(kded_ktouchpadenabler_SRCS
        ktouchpadenabler_daemon.cpp
    )

    kconfig_add_kcfg_files(kded_ktouchpadenabler_SRCS settings.kcfgc )

    kde4_add_plugin(kded_ktouchpadenabler
        ${kded_ktouchpadenabler_SRCS}
    )

    target_link_libraries(kded_ktouchpadenabler 
        
        ${KDE4_KDEUI_LIBS}
        ${X11_X11_LIB}
        ${X11_Xinput_LIB}
    )

    install(TARGETS kded_ktouchpadenabler DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES ktouchpadenabler.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)
endif ()
