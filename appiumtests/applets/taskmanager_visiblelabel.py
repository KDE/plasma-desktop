#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
import pyatspi

WIDGET_ID: Final = "org.kde.plasma.taskmanager"


class VisibleLabelTest(unittest.TestCase):
    """
    Tests that the label of the Task Manager is visible
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the widget and initialize the webdriver
        """
        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
        })
        options.set_capability("app", f"plasmoidviewer -a {WIDGET_ID} -c org.kde.panel -f vertical --size 120x500")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{WIDGET_ID}_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()

    def test_1_bug487023_group_dialog(self) -> None:
        """
        Tests the task manager label is visible and within panel bounds
        """

        environ = os.environ.copy()
        environ["GDK_BACKEND"] = "wayland"
        environ["AT_SPI_BUS_ADDRESS"] = "org:foo=bar"  # Disable atspi for the test window
        window_1 = subprocess.Popen(["python3", os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop", "desktoptest_testwindow.py")], env=environ)
        self.addCleanup(window_1.terminate)


        WebDriverWait(self.driver, 10).until(
            EC.visibility_of_element_located((AppiumBy.NAME, "Test Window-labelhint"))
        )
        r = self.driver.find_element(AppiumBy.NAME, "Test Window-labelhint").rect
        self.assertTrue(0 < r['x'] + r['width'] < 120)


if __name__ == '__main__':
    unittest.main()
