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
        KF6::CoreAddons
        KF6::I18n
        XCB::XCB
        PkgConfig::XKBCommon
        X11::xkbfile
        X11::X11
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(tastenbrett PRIVATE Qt6::X11Extras)
else()
    target_link_libraries(tastenbrett PRIVATE Qt::GuiPrivate)
endif()

install(TARGETS tastenbrett DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
