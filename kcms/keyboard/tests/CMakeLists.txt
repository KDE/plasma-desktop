macro(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               add_executable(${_testname}_test ${_testname}_test.cpp)
               ecm_mark_nongui_executable(${_testname}_test)
               ecm_mark_as_test(${_testname}_test)
               add_test(NAME kcm-keyboard-${_testname} COMMAND ${_testname}_test)
               target_link_libraries(${_testname}_test Qt::Test keyboard)
endmacro(KEYBOARD_DAEMON_UNIT_TESTS)

keyboard_daemon_unit_tests(xkb_rules)
keyboard_daemon_unit_tests(iso_codes)

add_executable(flags_test flags_test.cpp)
ecm_mark_nongui_executable(flags_test)
ecm_mark_as_test(flags_test)
add_test(NAME kcm-keyboard-flags_test COMMAND flags_test)
target_link_libraries(flags_test Qt::Test keyboard)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)

if( X11_XTest_FOUND )
   add_executable(keyboard_daemon_test keyboard_daemon_test.cpp ../keyboard_daemon.cpp ../layout_memory.cpp ../layout_memory_persister.cpp ../udev_helper.cpp ../xkb_helper.cpp ../xinput_helper.cpp ../keyboard_config.cpp ../keyboard_hardware.cpp)
   add_test(NAME kcm-keyboard-keyboard_daemon_test COMMAND keyboard_daemon_test)
   add_dependencies(keyboard_daemon_test kded_keyboard)
   ecm_mark_nongui_executable(keyboard_daemon_test)
   ecm_mark_as_test(keyboard_daemon_test)
   target_link_libraries(keyboard_daemon_test
                         Qt::X11Extras
                         Qt::Test
                         KF5::DBusAddons
                         KF5::GuiAddons
                         KF5::I18n
                         KF5::Notifications
                         KF5::WindowSystem
                         KF5::XmlGui
                         keyboard
)
   if (HAVE_XINPUT)
      target_link_libraries(keyboard_daemon_test X11::Xi)
   endif()

   if (HAVE_UDEV)
      target_link_libraries(keyboard_daemon_test ${UDEV_LIBS})
   endif()
endif()
