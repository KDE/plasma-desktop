macro(KEYBOARD_DAEMON_UNIT_TESTS _testname)
               add_executable(${_testname}_test ${_testname}_test.cpp ../${_testname}.cpp)
               ecm_mark_nongui_executable(${_testname}_test)
               ecm_mark_as_test(${_testname}_test)
               add_test(kcm-keyboard-${_testname} ${_testname}_test)
               target_link_libraries(${_testname}_test
                                     Qt5::Concurrent
                                     Qt5::Widgets
                                     Qt5::X11Extras
                                     Qt5::Xml
                                     Qt5::Test
                                     KF5::I18n
                                     KF5::KDELibs4Support
                                     ${X11_Xkbfile_LIB}
                                     ${X11_LIBRARIES}
)
endmacro(KEYBOARD_DAEMON_UNIT_TESTS)

macro(KEYBOARD_PREVIEW_UNIT_TESTS _testname)
		include_directories("..")
               set(test_SRCS ../preview/${_testname}.cpp ../preview/geometry_components.cpp ../xkb_rules.cpp)
               add_executable(${_testname}_test ${_testname}_test.cpp ${test_SRCS})
               ecm_mark_nongui_executable(${_testname}_test)
               ecm_mark_as_test(${_testname}_test)
               add_test(kcm-keyboard-${_testname} ${_testname}_test)
               target_link_libraries(${_testname}_test
                                     Qt5::Concurrent
                                     Qt5::Widgets
                                     Qt5::X11Extras
                                     Qt5::Xml
                                     Qt5::Test
                                     KF5::I18n
                                     KF5::KDELibs4Support
                                     ${X11_Xkbfile_LIB}
                                     ${X11_LIBRARIES}
		)
               set_source_files_properties(${test_SRCS} PROPERTIES COMPILE_FLAGS "-fexceptions")
endmacro(KEYBOARD_PREVIEW_UNIT_TESTS)


keyboard_daemon_unit_tests(xkb_rules)
keyboard_daemon_unit_tests(iso_codes)

find_package(Boost)

if(Boost_FOUND)
  ADD_DEFINITIONS(-DNEW_GEOMETRY=1)

  keyboard_preview_unit_tests(geometry_parser)
#  set_source_files_properties(${preview_SRCS} PROPERTIES COMPILE_FLAGS "-fexceptions")
endif(Boost_FOUND)

add_executable(flags_test flags_test.cpp ../flags.cpp ../x11_helper.cpp ../keyboard_config.cpp ../xkb_rules.cpp)
ecm_mark_nongui_executable(flags_test)
ecm_mark_as_test(flags_test)
add_test(kcm-keyboard-flags_test flags_test)
target_link_libraries(flags_test
                      Qt5::Concurrent
                      Qt5::X11Extras
                      Qt5::Xml
                      Qt5::Test
                      KF5::I18n
                      KF5::Plasma
                      KF5::KDELibs4Support
                      XCB::XCB
                      XCB::XKB
                      ${X11_Xkbfile_LIB}
                      ${X11_LIBRARIES}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)

if( X11_XTest_FOUND )
   add_executable(keyboard_daemon_test keyboard_daemon_test.cpp ../keyboard_daemon.cpp ../layout_memory.cpp ../layout_memory_persister.cpp ../flags.cpp ../x11_helper.cpp ../xkb_helper.cpp ../xinput_helper.cpp ../layout_tray_icon.cpp ../keyboard_config.cpp ../xkb_rules.cpp ../keyboard_hardware.cpp ../bindings.cpp ../layouts_menu.cpp)
   add_test(kcm-keyboard-keyboard_daemon_test keyboard_daemon_test)
   ecm_mark_nongui_executable(keyboard_daemon_test)
   ecm_mark_as_test(keyboard_daemon_test)
   target_link_libraries(keyboard_daemon_test
                         Qt5::Concurrent
                         Qt5::X11Extras
                         Qt5::Xml
                         Qt5::Test
                         KF5::ConfigCore
                         KF5::CoreAddons
                         KF5::DBusAddons
                         KF5::Plasma
                         KF5::KDELibs4Support
                         KF5::GlobalAccel
                         XCB::XCB
                         XCB::XKB
                         ${X11_Xkbfile_LIB}
                         ${X11_XTest_LIB}
                         ${X11_LIBRARIES}
)
   if(XiGetDevicePresence_FOUND)
      target_link_libraries(keyboard_daemon_test ${X11_Xinput_LIB})
   endif()
endif()
