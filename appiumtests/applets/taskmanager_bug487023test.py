#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
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

WIDGET_ID: Final = "org.kde.plasma.taskmanager"


class Bug487023Test(unittest.TestCase):
    """
    Tests for the task manager widget
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
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
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
        Tests the group dialog can be opened
        """
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Icons-and-Text Task Manager")))
        # Change "Clicking grouped task" to "Shows textual list"
        subprocess.check_call(["plasmawindowed", "--config"])
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Behavior"))).click()
        combobox_element: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Cycles through tasks")))
        combobox_element.click()
        combobox_element.click()
        combobox_element.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Shows textual list")))
        self.driver.find_element(AppiumBy.NAME, "Group only when the Task Manager is full").click()
        self.driver.find_element(AppiumBy.NAME, "OK").click()
        wait.until_not(lambda _: combobox_element.is_displayed())

        self.driver.quit()

        # Enable fatal warnings to check QML errors in the group dialog
        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false;org.kde.plasma.notificationmanager.warning=false;org.kde.plasma.taskmanager.warning=false",
        })
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

        environ = os.environ.copy()
        environ["GDK_BACKEND"] = "wayland"
        environ["AT_SPI_BUS_ADDRESS"] = "org:foo=bar"  # Disable atspi for the test window
        window_1 = subprocess.Popen(["python3", os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop", "desktoptest_testwindow.py")], env=environ)
        window_2 = subprocess.Popen(["python3", os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop", "desktoptest_testwindow.py")], env=environ)
        self.addCleanup(window_1.terminate)
        self.addCleanup(window_2.terminate)

        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Icons-and-Text Task Manager")))
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "python3"))).click()
        time.sleep(3)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)  # 2 tasks in the group dialog


if __name__ == '__main__':
    unittest.main()
