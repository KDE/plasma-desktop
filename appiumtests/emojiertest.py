#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import subprocess
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

KDE_VERSION: Final = 6


class EmojierTest(unittest.TestCase):
    """
    Tests for plasma-emojier
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        options = AppiumOptions()
        options.set_capability("app", f"plasma-emojier")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_emojier_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "plasma.emojier"])
        cls.driver.quit()

    def test_0_open(self) -> None:
        """
        Tests the app does not crash on opening
        @see https://bugs.kde.org/show_bug.cgi?id=478458
        """
        self.driver.find_element(AppiumBy.NAME, "Recent")
        self.driver.find_element(AppiumBy.NAME, "Clear History")

    def test_1_open_category(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "All").click()
        self.driver.find_element(AppiumBy.NAME, "grinning face").click()
        time.sleep(1)
        self.assertEqual(self.driver.get_clipboard_text(), "😀")

    def test_2_recent_usage(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "Recent").click()
        self.driver.find_element(AppiumBy.NAME, "grinning face")


if __name__ == '__main__':
    unittest.main()
