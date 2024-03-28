#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_plasmashell
from gi.repository import Gio, GLib
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait


class Bug472909Test(unittest.TestCase):
    """
    Kickoff is taking focus away after being closed with meta key
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()

        options = AppiumOptions()
        options.set_capability("app", "Root")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if os.environ["GDK_BACKEND"] == "wayland" and not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_bug472909_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_output(["kquitapp6", "plasmashell"], stderr=sys.stderr)
        cls.plasmashell.wait()
        if cls.kded:
            cls.kded.terminate()
            cls.kded.wait()
        if cls.kactivitymanagerd:
            cls.kactivitymanagerd.terminate()
            cls.kactivitymanagerd.wait()
        cls.driver.quit()

    def test_bug472909(self) -> None:
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Application Launcher")))

        # The driver doesn't support sending global keyboard shortcuts
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "allShortcutInfos")
        reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
        shortcut_list = reply.get_body().get_child_value(0)
        launcher_shortcut_id: str = ""
        for i in range(shortcut_list.n_children()):
            shortcut_info: GLib.Variant = shortcut_list.get_child_value(i)
            shortcut_id: str = shortcut_info.get_child_value(0).get_string()
            if not shortcut_id.startswith("activate application launcher"):
                continue
            launcher_shortcut_id = shortcut_id
            break
        self.assertGreater(len(launcher_shortcut_id), 0)

        # Start active indicator window
        test_window = subprocess.Popen([os.path.join(os.path.dirname(os.path.abspath(__file__)), "bug472909_activewindow.py")], stdout=sys.stderr, stderr=sys.stderr)
        self.addCleanup(test_window.terminate)

        def activate_shortcut() -> None:
            message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "invokeShortcut")
            message.set_body(GLib.Variant("(s)", [launcher_shortcut_id]))
            session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)

        # Activate the shortcut twice to test focus restoration
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Active Window")))
        activate_shortcut()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Inactive Window")))
        activate_shortcut()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Active Window")))

        test_window.terminate()
        test_window.wait()


if __name__ == '__main__':
    unittest.main()
