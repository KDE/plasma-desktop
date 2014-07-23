project("waylandtest")

set(waylandtest_SRCS
    main.cpp
    waylandtest.cpp
)
 
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS QuickWidgets)

ki18n_wrap_ui(waylandtest_SRCS waylandtest.ui)

add_executable(waylandtest ${waylandtest_SRCS})

target_link_libraries(waylandtest
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::X11Extras
    Qt5::Quick
    Qt5::QuickWidgets
    KF5::CoreAddons
#     KF5::KIOCore
#     KF5::Service
#     KF5::Notifications
    KF5::I18n
)

include(ECMMarkAsTest)
ecm_mark_as_test(waylandtest)
