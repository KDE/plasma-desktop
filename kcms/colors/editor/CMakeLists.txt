set(scheme_editor_SRCS
    kcolorschemeeditor.cpp
    scmeditordialog.cpp
    scmeditoroptions.cpp
    scmeditorcolors.cpp
    scmeditoreffects.cpp
    previewwidget.cpp
    setpreviewwidget.cpp
)

ki18n_wrap_ui(scheme_editor_SRCS
    scmeditordialog.ui
    scmeditoroptions.ui
    scmeditorcolors.ui
    scmeditoreffects.ui
    preview.ui
    setpreview.ui
)

add_executable(kcolorschemeeditor ${scheme_editor_SRCS})

target_link_libraries(kcolorschemeeditor
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::I18n
    KF5::CoreAddons
    KF5::NewStuff
    KF5::WindowSystem
)

install(TARGETS kcolorschemeeditor DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.kcolorschemeeditor.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
