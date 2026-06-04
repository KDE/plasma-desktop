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
