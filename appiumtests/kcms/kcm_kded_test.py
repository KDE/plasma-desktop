#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import unittest
import time
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_kded"


def loadedModules(session_bus: Gio.DBusConnection) -> list[str]:
    kded_reply: GLib.Variant = session_bus.call_sync(f"org.kde.kded{KDE_VERSION}", "/kded", f"org.kde.kded{KDE_VERSION}", "loadedModules", None, GLib.VariantType("(as)"), Gio.DBusSendMessageFlags.NONE, 1000)
    return kded_reply.get_child_value(0).unpack()

def name_has_owner(session_bus: Gio.DBusConnection | None, name: str) -> bool:
    """
    Whether the given name is available on session bus
    """
    if session_bus is None:
        session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)
    message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner")
    message.set_body(GLib.Variant("(s)", [name]))
    reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
    return reply and reply.get_signature() == 'b' and reply.get_body().get_child_value(0).get_boolean()

def start_kded() -> subprocess.Popen | None:
    session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
    kded = None
    if not name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
        kded = subprocess.Popen([f"kded{KDE_VERSION}"], stdout=sys.stderr, stderr=sys.stderr)
        kded_started: bool = False
        for _ in range(10):
            if name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
                kded_started = True
                break
            print(f"waiting for kded{KDE_VERSION} to appear on the dbus session")
            time.sleep(1)
        assert kded_started

    return kded

class KCMTest(unittest.TestCase):
    """
    Tests for kcm_kded
    The KCM is hidden in systemsettings by default so if it breaks, few people will notice it.
    This test makes sure the KCM still works.
    """

    driver: webdriver.Remote
    kded_process: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the KCM and initialize the webdriver
        """
        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")

        environment = {
            "LC_ALL": "en_US.UTF-8",
        }

        options.set_capability("environ", environment)

        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

        cls.kded_process = start_kded()

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
        cls.driver.find_element(AppiumBy.XPATH, "//*[@name='Cancel' and contains(@accessibility-id, 'Button')]").click()
        if cls.kded_process is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", f"kded{KDE_VERSION}"], stderr=sys.stderr)
            cls.kded_process.wait()
        cls.driver.quit()

    def test_0_start_stop_service(self) -> None:
        """
        Start/stop the accent color service
        """
        self.driver.find_element(AppiumBy.NAME, "Background Services")

        wait = WebDriverWait(self.driver, 30)

        START_BUTTON_NAME: Final = "Start Accent Color"
        STOP_BUTTON_NAME: Final = "Stop Accent Color"
        SERVICE_NAME: Final = "plasma_accentcolor_service"  # The id is from plasma-workspace

        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)

        # The service has X-KDE-Kded-autoload: true, so it is always loaded when kded is running.
        # Wait for autoloading to complete, then stop it to get into a known stopped state.
        wait.until(EC.element_to_be_clickable((AppiumBy.NAME, STOP_BUTTON_NAME))).click()
        wait.until(EC.element_to_be_clickable((AppiumBy.NAME, START_BUTTON_NAME)))
        wait.until(lambda _: SERVICE_NAME not in loadedModules(session_bus))
        self.assertNotIn(SERVICE_NAME, loadedModules(session_bus))

        # Start the service again, and verify it is loaded.
        wait.until(EC.element_to_be_clickable((AppiumBy.NAME, START_BUTTON_NAME))).click()
        stop_button: WebElement = wait.until(EC.element_to_be_clickable((AppiumBy.NAME, STOP_BUTTON_NAME)))
        wait.until(lambda _: SERVICE_NAME in loadedModules(session_bus))
        self.assertIn(SERVICE_NAME, loadedModules(session_bus))

        # Stop the service again to make sure it can be stopped again after being started.
        stop_button.click()
        wait.until(EC.element_to_be_clickable((AppiumBy.NAME, START_BUTTON_NAME)))
        wait.until(lambda _: SERVICE_NAME not in loadedModules(session_bus))
        self.assertNotIn(SERVICE_NAME, loadedModules(session_bus))

    def test_1_toggle_automatically_loading_service(self) -> None:
        """
        Toggle automatically loading the accent color service
        """
        self.driver.find_element(AppiumBy.NAME, "Disable automatically loading Accent Color on startup").click()
        self.driver.find_element(AppiumBy.NAME, "Apply").click()

        wait = WebDriverWait(self.driver, 30)
        wait.until(lambda _: subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kded5rc", "--group", "Module-plasma_accentcolor_service", "--key", "autoload"]).decode(encoding="utf-8").strip() == "false")

        self.driver.find_element(AppiumBy.NAME, "Enable automatically loading Accent Color on startup").click()
        self.driver.find_element(AppiumBy.NAME, "Apply").click()
        wait.until(lambda _: subprocess.check_output([f"kreadconfig{KDE_VERSION}", "--file", "kded5rc", "--group", "Module-plasma_accentcolor_service", "--key", "autoload"]).decode(encoding="utf-8").strip() == "true")


if __name__ == '__main__':
    assert os.getenv("USE_CUSTOM_BUS") == "1"
    unittest.main()
