set(CMAKE_AUTORCC ON)

pkg_check_modules(XKBCommon REQUIRED IMPORTED_TARGET xkbcommon)

add_executable(tastenbrett
    application.cpp
    doodad.cpp
    geometry.cpp
    key.cpp
    main.cpp
    outline.cpp
    qml.qrc
    row.cpp
    section.cpp
    shape.cpp
    xkbobject.cpp
)

target_compile_definitions(tastenbrett
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(tastenbrett
    PRIVATE
        Qt::Core
        Qt::Quick
        Qt::Gui
        Qt::X11Extras
        KF5::CoreAddons
        KF5::I18n
        XCB::XCB
        PkgConfig::XKBCommon
        X11::xkbfile
        X11::X11
)

install(TARGETS tastenbrett DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
