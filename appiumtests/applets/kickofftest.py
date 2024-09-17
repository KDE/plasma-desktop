#!/usr/bin/env python3

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

import os
import subprocess
import sys
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import start_kactivitymanagerd

WIDGET_ID: Final = "org.kde.plasma.kickoff"


class KickoffTests(unittest.TestCase):
    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        cls.kactivitymanagerd = start_kactivitymanagerd()

        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
        })
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    @classmethod
    def tearDownClass(self) -> None:
        self.driver.quit()
        if self.kactivitymanagerd is not None:
            self.kactivitymanagerd.terminate()
            self.kactivitymanagerd.wait()

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{WIDGET_ID}_#{self.id()}.png")

    def test_0_open(self) -> None:
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | All Applications]")

    def test_1_search(self) -> None:
        self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("12345+67890")
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | 80235]")


if __name__ == '__main__':
    unittest.main()
