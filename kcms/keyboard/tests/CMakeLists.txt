macro(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               add_executable(${_testname}_test ${_testname}_test.cpp ../${_testname}.cpp ../debug.cpp)
               ecm_mark_nongui_executable(${_testname}_test)
               ecm_mark_as_test(${_testname}_test)
               add_test(NAME kcm-keyboard-${_testname} COMMAND ${_testname}_test)
               target_link_libraries(${_testname}_test
                                     Qt::Concurrent
                                     Qt::Widgets
                                     Qt::X11Extras
                                     Qt::Xml
                                     Qt::Test
                                     KF5::I18n
                                     KF5::WindowSystem
                                     X11::xkbfile
                                     X11::X11
)
endmacro(KEYBOARD_DAEMON_UNIT_TESTS)

keyboard_daemon_unit_tests(xkb_rules)
keyboard_daemon_unit_tests(iso_codes)

add_executable(flags_test flags_test.cpp ../flags.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp ../debug.cpp ../layoutnames.cpp)
ecm_mark_nongui_executable(flags_test)
ecm_mark_as_test(flags_test)
add_test(NAME kcm-keyboard-flags_test COMMAND flags_test)
target_link_libraries(flags_test
                      Qt::Concurrent
                      Qt::X11Extras
                      Qt::Xml
                      Qt::Test
                      Qt::DBus
                      Qt::Widgets
                      KF5::GuiAddons
                      KF5::I18n
                      KF5::Plasma
                      KF5::WindowSystem
                      XCB::XCB
                      XCB::XKB
                      X11::xkbfile
                      X11::X11
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)

if( X11_XTest_FOUND )
   add_executable(keyboard_daemon_test keyboard_daemon_test.cpp ../keyboard_daemon.cpp ../layout_memory.cpp ../layout_memory_persister.cpp ../flags.cpp ../udev_helper.cpp ../x11_helper.cpp ../xkb_helper.cpp ../xinput_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp ../keyboard_hardware.cpp ../bindings.cpp ../debug.cpp ../layoutnames.cpp)
   add_test(NAME kcm-keyboard-keyboard_daemon_test COMMAND keyboard_daemon_test)
   add_dependencies(keyboard_daemon_test kded_keyboard)
   ecm_mark_nongui_executable(keyboard_daemon_test)
   ecm_mark_as_test(keyboard_daemon_test)
   target_link_libraries(keyboard_daemon_test
                         Qt::Concurrent
                         Qt::X11Extras
                         Qt::Xml
                         Qt::Test
                         KF5::ConfigCore
                         KF5::CoreAddons
                         KF5::DBusAddons
                         KF5::GuiAddons
                         KF5::I18n
                         KF5::Notifications
                         KF5::Plasma
                         KF5::GlobalAccel
                         KF5::WindowSystem
                         KF5::XmlGui
                         XCB::XCB
                         XCB::XKB
                         X11::xkbfile
                         ${X11_XTest_LIB}
                         X11::X11
)
   if (HAVE_XINPUT)
      target_link_libraries(keyboard_daemon_test X11::Xi)
   endif()

   if (HAVE_UDEV)
      target_link_libraries(keyboard_daemon_test ${UDEV_LIBS})
   endif()
endif()
