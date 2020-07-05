set(kcm_keyboard_layoutmodel_SRCS
    layout_list_models.cpp
    layout_list_model_base.cpp
    layout_list_model_xkb.cpp
    layout_list_sort_by_priority_proxy_model.cpp
    layout_list_filter_duplicates_proxy_model.cpp
    layout_list_xkb_expand_proxy_model.cpp
    layout_list_model_fcitx.cpp
    layout_list_concat_proxy_model.cpp
    layout_list_filter_source_proxy_model.cpp
    layout_list_model_selected.cpp
    layout_list_sort_filter_proxy_model.cpp
    layout_list_filter_disabled_proxy_model.cpp
    layout_list_current_proxy_model.cpp

    input_sources.cpp

    xkb_layout_config_model.cpp
    fcitx_im_config_model.cpp

    ../xkb_rules.cpp
)

ecm_qt_declare_logging_category(kcm_keyboard_layoutmodel_SRCS
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
    DEFAULT_SEVERITY Warning)

add_library(kcm_keyboard_layoutmodel STATIC ${kcm_keyboard_layoutmodel_SRCS})

target_link_libraries(kcm_keyboard_layoutmodel
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::QuickAddons
    KF5::Declarative
    KF5::KCMUtils
    KF5::I18n
    KF5::GlobalAccel
    KF5::WindowSystem
    KF5::IconThemes
    KF5::XmlGui
    KF5::Plasma
    KF5::ItemModels
    KF5::GuiAddons
    Qt5::Quick
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::DBus
    XCB::XCB
    XCB::XKB
    ${X11_LIBRARIES}
    ${X11_Xkbfile_LIB}
    kcm_keyboard_fcitx
)

target_include_directories(kcm_keyboard_layoutmodel PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

configure_file(../config-keyboard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-keyboard.h)
