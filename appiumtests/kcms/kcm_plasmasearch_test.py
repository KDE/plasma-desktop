#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_plasmasearch"


class KCMTest(unittest.TestCase):
    """
    Tests for kcm_plasmasearch
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the KCM and initialize the webdriver
        """
        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
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
        """
        Tests the plugin list is loaded
        @see https://bugs.kde.org/show_bug.cgi?id=476702
        """
        self.driver.find_element(AppiumBy.NAME, "Configure KRunner…")
        self.driver.find_element(AppiumBy.NAME, "Applications")
        self.driver.find_element(AppiumBy.NAME, "Remove from favorites")
        self.driver.find_element(AppiumBy.NAME, "File Search")


if __name__ == '__main__':
    unittest.main()
