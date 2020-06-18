set(scheme_editor_SRCS
    kcolorschemeeditor.cpp
#     previewbutton.cpp
#     previewbridge.cpp
#     previewclient.cpp
#     previewitem.cpp
#     previewsettings.cpp
#     plugin.cpp
#     buttonsmodel.cpp
    resources.qrc
)

add_executable(kcolorschemeeditor ${scheme_editor_SRCS})

target_link_libraries(kcolorschemeeditor
    Qt5::DBus
    Qt5::Quick
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::I18n
    KDecoration2::KDecoration
    KDecoration2::KDecoration2Private
#     KF5::NewStuff
#     KF5::QuickAddons
#     KF5::Service
)

install(TARGETS kcolorschemeeditor DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.kcolorschemeeditor.desktop  DESTINATION  ${KDE_INSTALL_APPDIR})
