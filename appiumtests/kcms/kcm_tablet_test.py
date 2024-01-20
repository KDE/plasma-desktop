#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2021 Red Hat Inc.
# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_tablet"


class KCMTest(unittest.TestCase):
    """
    Tests for kcm_tablet
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")
        options.set_capability("environ", {
            "DBUS_SYSTEM_BUS_ADDRESS": os.environ['DBUS_SYSTEM_BUS_ADDRESS'],
            "DBUS_SESSION_BUS_ADDRESS": os.environ['DBUS_SESSION_BUS_ADDRESS'],
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "0",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.dbus.integration.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{KCM_ID}_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()

    def test_0_open(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "One by Wacom (M)")


if __name__ == '__main__':
    unittest.main()
