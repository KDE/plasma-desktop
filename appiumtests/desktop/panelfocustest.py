#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Niccolò Venerandi <niccolo@venerandi.com>
# SPDX-License-Identifier: MIT

import functools
import os
import pathlib
import stat
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
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from desktoptest import start_plasmashell

class PanelFocusTest(unittest.TestCase):
    """
    Tests for panel focus
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver
        """
        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()
        options = AppiumOptions()
        options.set_capability("app", "Root")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_plasmashell_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()

    def test_0_peek_at_desktop_not_giving_panel_focus(self) -> None:
        """
        This clicks on the Peek at Desktop element and checks that the panel has
        not gained keyboard focus by checking for the invisibility of the panel
        focus indicator.
        """
        el = self.driver.find_element(by=AppiumBy.NAME, value="Peek at Desktop")
        el.click()
        WebDriverWait(self.driver, 6).until(
            EC.visibility_of_element_located((AppiumBy.NAME, "Minimize All Applet Active Indicator"))
        )
        WebDriverWait(self.driver, 6).until(
            EC.invisibility_of_element_located((AppiumBy.NAME, "Panel Focus Indicator"))
        )
        el.click()

    def test_1_setting_focus_shortcut(self) -> None:
        """
        We set a shortcut for the panel, we activate it, and we check that the panel
        focus indicator is indeed visible.
        """

        # The driver doesn't support sending global keyboard shortcuts
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "allShortcutInfos")
        reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
        shortcut_list = reply.get_body().get_child_value(0)
        before = set(shortcut_list.get_child_value(i).get_child_value(0).get_string()
                     for i in range(shortcut_list.n_children()))

        actions = ActionChains(self.driver)
        actions.context_click(self.driver.find_element(by=AppiumBy.NAME, value="Peek at Desktop"))
        # Wait for the context menu to appear
        actions.pause(0.4)
        actions.send_keys(Keys.ARROW_UP)
        actions.pause(0.1)
        actions.send_keys(Keys.ENTER)
        actions.perform()

        setter = self.driver.find_element(by=AppiumBy.NAME, value="Focus Shortcut Setter")
        actions.click(setter)
        actions.key_down(Keys.ALT)
        actions.send_keys("a")
        actions.key_up(Keys.ALT)
        # Wait for the shortcut input button to set the new shortcut
        actions.pause(1.5)
        actions.send_keys(Keys.ESCAPE)
        actions.perform()

        actions.pause(0.5)
        actions.context_click()
        # Wait for the context menu to appear
        actions.pause(0.1)
        actions.send_keys(Keys.ARROW_UP)
        actions.pause(0.1)
        actions.send_keys(Keys.ENTER)
        actions.pause(1)
        actions.perform()

        message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "allShortcutInfos")
        reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
        shortcut_list = reply.get_body().get_child_value(0)
        after = set(shortcut_list.get_child_value(i).get_child_value(0).get_string()
                     for i in range(shortcut_list.n_children()))
        difference = after - before

        self.assertTrue(len(difference) == 1)
        launcher_shortcut_id = difference.pop()

        message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "invokeShortcut")
        message.set_body(GLib.Variant("(s)", [launcher_shortcut_id]))
        session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)

        WebDriverWait(self.driver, 6).until(
            EC.visibility_of_element_located((AppiumBy.NAME, "Panel Focus Indicator"))
        )

if __name__ == '__main__':
    unittest.main()
