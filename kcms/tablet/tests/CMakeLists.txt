ecm_add_test(
        TEST_NAME tst_calibrationtool
        tst_calibrationtool.cpp ../calibrationtool.cpp ${CMAKE_CURRENT_BINARY_DIR}/../logging.cpp)
target_link_libraries(tst_calibrationtool PRIVATE Qt::Test KF6::ConfigCore Canberra::Canberra kwindevices)
target_include_directories(tst_calibrationtool PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..) # ECM logging from the KCM