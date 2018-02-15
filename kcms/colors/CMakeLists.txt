# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmcolors\")

set(scheme_editor_SRCS
    kcolorschemeeditor.cpp
    scmeditordialog.cpp
    scmeditoroptions.cpp
    scmeditorcolors.cpp
    scmeditoreffects.cpp
    previewwidget.cpp
    setpreviewwidget.cpp
    colorscm.cpp
    ../krdb/krdb.cpp
    )

set(klauncher_xml ${KINIT_DBUS_INTERFACES_DIR}/kf5_org.kde.KLauncher.xml)

ki18n_wrap_ui(scheme_editor_SRCS
    colorsettings.ui
    scmeditordialog.ui
    scmeditoroptions.ui
    scmeditorcolors.ui
    scmeditoreffects.ui
    preview.ui
    setpreview.ui
    )

qt5_add_dbus_interface(scheme_editor_SRCS ${klauncher_xml} klauncher_iface)

add_executable(kcolorschemeeditor ${scheme_editor_SRCS})

target_link_libraries(kcolorschemeeditor
    KF5::KCMUtils
    KF5::GuiAddons
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    KF5::NewStuff
    KF5::WindowSystem
    )
if(X11_FOUND)
    target_link_libraries(kcolorschemeeditor ${X11_LIBRARIES} Qt5::X11Extras)
endif()
install(TARGETS kcolorschemeeditor  DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.kcolorschemeeditor.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})

# ----------------

set(kcm_colors_SRCS
    ../krdb/krdb.cpp
    colorscm.cpp
    scmeditordialog.cpp
    scmeditoroptions.cpp
    scmeditorcolors.cpp
    scmeditoreffects.cpp
    previewwidget.cpp
    setpreviewwidget.cpp
    )


qt5_add_dbus_interface(kcm_colors_SRCS ${klauncher_xml} klauncher_iface)

ki18n_wrap_ui(kcm_colors_SRCS
    colorsettings.ui
    scmeditordialog.ui
    scmeditoroptions.ui
    scmeditorcolors.ui
    scmeditoreffects.ui
    preview.ui
    setpreview.ui)

add_library(kcm_colors MODULE ${kcm_colors_SRCS})
target_link_libraries(kcm_colors
    KF5::KCMUtils
    KF5::GuiAddons
    KF5::I18n
    KF5::KIOCore
    KF5::CoreAddons
    Qt5::DBus
    KF5::NewStuff
    KF5::WindowSystem)

if(X11_FOUND)
    target_link_libraries(kcm_colors ${X11_LIBRARIES} Qt5::X11Extras)
endif()

install(TARGETS kcm_colors  DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES colors.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES colorschemes.knsrc DESTINATION  ${KDE_INSTALL_CONFDIR} )

# built-in color schemes
file(GLOB schemefiles schemes/*.colors)
install( FILES ${schemefiles} DESTINATION ${KDE_INSTALL_DATADIR}/color-schemes )
