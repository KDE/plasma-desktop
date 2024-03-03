# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_test(
    NAME desktoptest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/desktoptest.py --failfast
)
set_tests_properties(desktoptest PROPERTIES TIMEOUT 300 ENVIRONMENT "TEST_WITHOUT_GLOBAL_SHORTCUT=0;CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR};KACTIVITYMANAGERD_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/kactivitymanagerd")

add_test(
    NAME folderviewtest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/folderviewtest.py --failfast
)
set_tests_properties(folderviewtest PROPERTIES TIMEOUT 300 ENVIRONMENT "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR};KACTIVITYMANAGERD_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/kactivitymanagerd;TEST_WITH_KWIN_WAYLAND=0")

add_test(
    NAME bug472909test_wayland
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/bug472909.py --failfast
)

add_test(
    NAME bug476968test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/bug476968.py --failfast
)

add_test(
    NAME bug477220test
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/bug477220.py --failfast
)
set_tests_properties(bug472909test_wayland bug476968test bug477220test PROPERTIES TIMEOUT 600 ENVIRONMENT "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR};KACTIVITYMANAGERD_PATH=${KDE_INSTALL_FULL_LIBEXECDIR}/kactivitymanagerd;GDK_BACKEND=wayland")
