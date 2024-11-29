# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_test(
    NAME kcm_kded_test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcm_kded_test.py
)
set_tests_properties(kcm_kded_test PROPERTIES TIMEOUT 120 ENVIRONMENT "USE_CUSTOM_BUS=1")

add_test(
    NAME kcm_keys_test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcm_keys_test.py --failfast
)
set_tests_properties(kcm_keys_test PROPERTIES TIMEOUT 120 ENVIRONMENT "LC_ALL=en_US.UTF-8;TEST_WITH_KWIN_WAYLAND=0;USE_CUSTOM_BUS=1;KGLOBALACCELD_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/kglobalacceld")

add_test(
    NAME kcm_plasmasearch_test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/kcm_plasmasearch_test.py
)
set_tests_properties(kcm_plasmasearch_test PROPERTIES TIMEOUT 120)
