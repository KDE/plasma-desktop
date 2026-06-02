macro(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               add_executable(${_testname}_test ${_testname}_test.cpp ../${_testname}.cpp ../debug.cpp)
               ecm_mark_nongui_executable(${_testname}_test)
               ecm_mark_as_test(${_testname}_test)
               add_test(NAME kcm-keyboard-${_testname} COMMAND ${_testname}_test)
               target_link_libraries(${_testname}_test
                                     Qt::Concurrent
                                     Qt::Widgets
                                     Qt::Xml
                                     Qt::Test
                                     KF6::I18n
                                     KF6::WindowSystem
                                     X11::xkbfile
                                     X11::X11
                                     PkgConfig::XKBREGISTRY
               )
               target_link_libraries(${_testname}_test Qt::GuiPrivate)
endmacro(KEYBOARD_DAEMON_UNIT_TESTS)

keyboard_daemon_unit_tests(xkb_rules)
keyboard_daemon_unit_tests(iso_codes)

add_executable(flags_test flags_test.cpp)
ecm_mark_nongui_executable(flags_test)
ecm_mark_as_test(flags_test)
add_test(NAME kcm-keyboard-flags_test COMMAND flags_test)
target_link_libraries(flags_test
                      kcm_keyboard_static
                      Qt::Test
                      Qt::DBus
                      Qt::Widgets
                      KF6::GuiAddons
                      KF6::KCMUtils
                      KF6::I18n
                      Plasma::Plasma
                      KF6::WindowSystem
                      XCB::XCB
                      XCB::XKB
                      PkgConfig::XKBREGISTRY
                      X11::xkbfile
                      X11::X11
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)
