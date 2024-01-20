# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_test(
    NAME kcm_kded_test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcm_kded_test.py
)
set_tests_properties(kcm_kded_test PROPERTIES TIMEOUT 120 ENVIRONMENT "USE_CUSTOM_BUS=1")

add_test(
    NAME kcm_plasmasearch_test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcm_plasmasearch_test.py
)
set_tests_properties(kcm_plasmasearch_test PROPERTIES TIMEOUT 120)

execute_process(COMMAND gcc -print-file-name=libasan.so OUTPUT_VARIABLE LIBASAN_PATH ECHO_OUTPUT_VARIABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND gcc -print-file-name=libumockdev-preload.so.0 OUTPUT_VARIABLE LIBUMOCKDEV_PATH ECHO_OUTPUT_VARIABLE OUTPUT_STRIP_TRAILING_WHITESPACE)
add_test(
    NAME kcm_tablet_test
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/kcm_tablet_test_wrapper.py
)
set_tests_properties(kcm_tablet_test PROPERTIES TIMEOUT 120 ENVIRONMENT "LD_PRELOAD=${LIBASAN_PATH}:${LIBUMOCKDEV_PATH};LIBASAN_PATH=${LIBASAN_PATH};LIBUMOCKDEV_PATH=${LIBUMOCKDEV_PATH}")
