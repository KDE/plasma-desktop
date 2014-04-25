
set(kcm_smserver_PART_SRCS kcmsmserver.cpp smserverconfigimpl.cpp )


kde4_add_ui_files(kcm_smserver_PART_SRCS smserverconfigdlg.ui )

add_library(kcm_smserver MODULE ${kcm_smserver_PART_SRCS})


target_link_libraries(kcm_smserver
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

install(TARGETS kcm_smserver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcmsmserver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

