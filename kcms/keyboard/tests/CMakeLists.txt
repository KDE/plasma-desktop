ecm_add_test(flags_test.cpp LINK_LIBRARIES Qt::Test Qt::Gui kcm_keyboard_static)
ecm_add_test(xkb_rules_test.cpp LINK_LIBRARIES Qt::Test Qt::Gui kcm_keyboard_static)
ecm_add_test(iso_codes_test.cpp ../iso_codes.cpp TEST_NAME test_iso_codes LINK_LIBRARIES Qt::Test Qt::Gui KF6::I18n kcm_keyboard_static)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.1.1.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.1.1.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.bad.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.bad.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/base.xml ${CMAKE_CURRENT_BINARY_DIR}/config/base.xml COPYONLY)
