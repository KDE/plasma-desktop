#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from gi.repository import GLib
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import name_has_owner

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_keys"
KGLOBALACCELD_PATH: Final = os.getenv("KGLOBALACCELD_PATH", "/usr/libexec/kglobalacceld")


class KCMTest(unittest.TestCase):
    """
    Tests for kcm_keys
    """

    driver: webdriver.Remote
    kded: subprocess.Popen | None = None
    kglobalacceld: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        # Start KGlobalAccel daemon service
        if not name_has_owner(None, "org.kde.kglobalaccel"):
            cls.kglobalacceld = subprocess.Popen([KGLOBALACCELD_PATH], stdout=sys.stderr, stderr=sys.stderr)

        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.qml.typeresolution.cycle.warning=false;qt.qpa.wayland.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        pass

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.find_element(AppiumBy.XPATH, "//*[@name='Cancel' and contains(@accessibility-id, 'Button')]").click()
        if cls.kded is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", f"kded{KDE_VERSION}"], stderr=sys.stderr)
            cls.kded.wait()
        if cls.kglobalacceld is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", f"kglobalaccel"], stderr=sys.stderr)
            cls.kglobalacceld.wait()
        cls.driver.quit()

    def test_0_open(self) -> None:
        # Placeholder
        self.driver.find_element(AppiumBy.NAME, "Select an item from the list to view its shortcuts here")
        # Sidebar
        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='Applications' and contains(@accessibility-id, 'ListSectionHeader')]")
        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='Common Actions' and contains(@accessibility-id, 'ListSectionHeader')]")

    def test_1_application_shortcuts(self) -> None:
        """
        List some applications' shortcuts
        """
        self.assertTrue(name_has_owner(None, "org.kde.kglobalaccel"))
        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='KRunner']").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Run command on clipboard contents:Alt+Shift+F2").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Default shortcut Alt+Shift+F2 is enabled.")

        self.driver.find_element(by=AppiumBy.XPATH, value="//list_item[@name='Emoji Selector']").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Launch:")
        self.driver.find_element(by=AppiumBy.NAME, value="Meta+.")
        self.driver.find_element(by=AppiumBy.NAME, value="Meta+Ctrl+Alt+Shift+Space")

        # Test if the config is saved
        self.driver.find_element(by=AppiumBy.NAME, value="Add custom shortcut").click()
        subprocess.check_call(["xdotool", "key", "Ctrl+0x002e"])  # https://gitlab.com/nokun/gestures/-/wikis/xdotool-list-of-key-codes
        delete_button = self.driver.find_element(by=AppiumBy.NAME, value="Delete shortcut")
        apply_button = self.driver.find_element(AppiumBy.XPATH, "//*[@name='Apply' and contains(@accessibility-id, 'Button')]")
        apply_button.click()
        wait = WebDriverWait(self.driver, 10)
        wait.until(lambda _: "Ctrl+." in subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kglobalshortcutsrc", "--group", "services", "--group", "org.kde.plasma.emojier.desktop", "--key", "_launch"]).decode(encoding="utf-8").strip())

        # Remove the custom shortcut and check again
        delete_button.click()
        apply_button.click()
        wait.until_not(lambda _: "Ctrl+." in subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kglobalshortcutsrc", "--group", "services", "--group", "org.kde.plasma.emojier.desktop", "--key", "_launch"]).decode(encoding="utf-8").strip())

    def test_2_add_new(self) -> None:
        # Open the dialog
        self.driver.find_element(AppiumBy.NAME, "Add New").click()
        self.driver.find_element(AppiumBy.NAME, "Add new command or script").click()
        print(self.driver.page_source, file=sys.stderr)
        self.driver.find_element(AppiumBy.NAME, "Add Command").click()

        # Add a new command
        command = "echo appiumtest > /dev/null"
        self.driver.find_element(AppiumBy.NAME, "Command").send_keys(command)
        self.driver.find_element(AppiumBy.NAME, "Add").click()

        # Assign a shortcut
        self.driver.find_element(AppiumBy.NAME, f"{command}:")
        self.driver.find_element(AppiumBy.NAME, "No default shortcuts")
        self.driver.find_element(by=AppiumBy.NAME, value="Add custom shortcut").click()
        subprocess.check_call(["xdotool", "key", "Ctrl+0x002e"])
        delete_button = self.driver.find_element(by=AppiumBy.NAME, value="Delete shortcut")
        apply_button = self.driver.find_element(AppiumBy.XPATH, "//*[@name='Apply' and contains(@accessibility-id, 'Button')]")
        apply_button.click()

        # Test if the config is saved
        custom_command_desktop_file = "net.local.echo.desktop"
        wait = WebDriverWait(self.driver, 10)
        wait.until(lambda _: subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kglobalshortcutsrc", "--group", "services", "--group", custom_command_desktop_file, "--key", "_launch"]).decode(encoding="utf-8").strip() == "Ctrl+.")
        with open(os.path.join(GLib.get_user_data_dir(), "applications", custom_command_desktop_file), encoding="utf-8") as file_handler:
            for line in file_handler:
                if line.startswith("Exec="):
                    self.assertIn(command, line)
                if line.startswith("X-KDE-GlobalAccel-CommandShortcut"):
                    self.assertIn("true", line)

        # Remove the custom shortcut and check again
        delete_button.click()
        apply_button.click()
        wait.until(lambda _: subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kglobalshortcutsrc", "--group", "services", "--group", custom_command_desktop_file, "--key", "_launch"]).decode(encoding="utf-8").strip() == "")
        self.assertTrue(os.path.exists(os.path.join(GLib.get_user_data_dir(), "applications", custom_command_desktop_file)))

        # Remove the custom command
        self.driver.find_element(by=AppiumBy.NAME, value="Remove all shortcuts for ").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Undo deletion").click()
        self.driver.find_element(by=AppiumBy.NAME, value="Remove all shortcuts for ").click()
        apply_button.click()
        wait.until_not(lambda _: os.path.exists(os.path.join(GLib.get_user_data_dir(), "applications", custom_command_desktop_file)))


if __name__ == '__main__':
    assert os.getenv("LC_ALL") == "en_US.UTF-8"
    assert os.getenv("TEST_WITH_KWIN_WAYLAND") == "0"  # On Wayland kglobalshortcutsrc is not updated
    unittest.main()
