# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

add_test(
    NAME taskmanager_groupdialogtest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/taskmanager_bug487023test.py
)
set_tests_properties(taskmanager_groupdialogtest PROPERTIES TIMEOUT 120)
