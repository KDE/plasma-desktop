# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

if(NOT BUILD_TESTING OR NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    return()
endif()
find_package(SeleniumWebDriverATSPI)
set_package_properties(SeleniumWebDriverATSPI PROPERTIES
    DESCRIPTION "Server component for selenium tests using Linux accessibility infrastructure"
    PURPOSE "Needed for GUI tests"
    URL "https://invent.kde.org/sdk/selenium-webdriver-at-spi"
    TYPE OPTIONAL
)
if(NOT SeleniumWebDriverATSPI_FOUND AND NOT DEFINED ENV{KDECI_BUILD})
    return()
endif()

add_subdirectory(applets)
add_subdirectory(desktop)
add_subdirectory(kcms)

add_test(
    NAME emojiertest
    COMMAND selenium-webdriver-at-spi-run ${CMAKE_CURRENT_SOURCE_DIR}/emojiertest.py --failfast
)
set_tests_properties(emojiertest PROPERTIES TIMEOUT 300 ENVIRONMENT "TEST_WITH_KWIN_WAYLAND=0")
