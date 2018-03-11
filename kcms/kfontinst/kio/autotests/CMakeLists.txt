find_package(Qt5 CONFIG REQUIRED Test)
include(ECMAddTests)

ecm_add_tests(
   test_kio_fonts.cpp
   LINK_LIBRARIES Qt5::Test KF5::KIOWidgets Qt5::DBus
)

