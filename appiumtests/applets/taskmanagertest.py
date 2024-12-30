#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import (DesktopFileWrapper, start_kactivitymanagerd)

WIDGET_ID: Final = "org.kde.plasma.taskmanager"
KDE_VERSION: Final = 6


class Bug487023Test(unittest.TestCase):
    """
    Tests for the task manager widget
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    wrapper: DesktopFileWrapper

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the widget and initialize the webdriver
        """
        cls.wrapper = DesktopFileWrapper()
        cls.wrapper.create()
        cls.kactivitymanagerd = start_kactivitymanagerd()

        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "0",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false;org.kde.plasma.notificationmanager.warning=false;org.kde.plasma.taskmanager.debug=true",
        })
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

        os.environ["GDK_BACKEND"] = "wayland"
        os.environ["AT_SPI_BUS_ADDRESS"] = "org:foo=bar"  # Disable atspi for the test window

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
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "plasmawindowed"])
        for _ in range(10):
            try:
                subprocess.check_call(["pidof", "plasmawindowed"])
            except subprocess.CalledProcessError:
                break
            time.sleep(1)
        cls.driver.quit()
        if cls.kactivitymanagerd is not None:
            cls.kactivitymanagerd.terminate()
            cls.kactivitymanagerd.wait()

    def test_0_open(self) -> None:
        """
        Open the widget
        """
        self.driver.find_element(AppiumBy.NAME, "Icons-and-Text Task Manager")

    def test_1_badge_count(self) -> None:
        """
        Can list running windows and show badge counts based on app id
        """
        test_window = subprocess.Popen([self.wrapper.application_path], stdout=sys.stderr, stderr=sys.stderr)
        assert test_window.poll() is None
        self.addCleanup(test_window.kill)

        # Wait until the window appears in the widget
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Test Window")))

        # Set badge count and match again
        changed_properties = GLib.Variant('a{sv}', {
            "count": GLib.Variant('x', 123),
            "count-visible": GLib.Variant('b', True),
        })
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{self.wrapper.APPLICATION_ID}.desktop"), changed_properties))
        badge_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "123")))

        # Hide the badge count
        changed_properties = GLib.Variant('a{sv}', {
            "count-visible": GLib.Variant('b', False),
        })
        session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{self.wrapper.APPLICATION_ID}.desktop"), changed_properties))
        wait.until_not(lambda _: badge_element.is_displayed())

    def test_2_bug487023_group_dialog(self) -> None:
        """
        Tests the group dialog can be opened
        """
        # Change "Clicking grouped task" to "Shows textual list"
        wait = WebDriverWait(self.driver, 30)
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

        window_1 = subprocess.Popen(["python3", self.wrapper.application_path])
        window_2 = subprocess.Popen(["python3", self.wrapper.application_path])
        self.addCleanup(window_1.kill)
        self.addCleanup(window_2.kill)

        wait.until(EC.presence_of_element_located((AppiumBy.NAME, self.wrapper.APPLICATION_NAME))).click()
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)  # 2 tasks in the group dialog


if __name__ == '__main__':
    unittest.main()
