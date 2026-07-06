ecm_add_test(
        TEST_NAME tst_inputdevice
        tst_inputdevice.cpp tst_inputdevice.cpp)
target_link_libraries(tst_inputdevice PRIVATE Qt::Test kwindevices)
