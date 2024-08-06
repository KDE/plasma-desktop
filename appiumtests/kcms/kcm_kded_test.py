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
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import start_kded

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_kded"


def loadedModules(session_bus: Gio.DBusConnection) -> list[str]:
    kded_reply: GLib.Variant = session_bus.call_sync(f"org.kde.kded{KDE_VERSION}", "/kded", f"org.kde.kded{KDE_VERSION}", "loadedModules", None, GLib.VariantType("(as)"), Gio.DBusSendMessageFlags.NONE, 1000)
    return kded_reply.get_child_value(0).unpack()


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
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "1",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.qml.typeresolution.cycle.warning=false;qt.qpa.wayland.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
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
        if cls.kded_process is not None:
            cls.kded_process.terminate()
            cls.kded_process.wait()
        cls.driver.quit()

    def test_0_start_stop_service(self) -> None:
        """
        Start/stop the accent color service
        """
        self.driver.find_element(AppiumBy.NAME, "Background Services")
        self.driver.find_element(AppiumBy.NAME, "Start Accent Color").click()

        wait = WebDriverWait(self.driver, 30)
        stop_button: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Stop Accent Color")))

        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        self.assertIn("plasma_accentcolor_service", loadedModules(session_bus))  # The service id is from plasma-workspace

        stop_button.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Start Accent Color")))
        self.assertNotIn("plasma_accentcolor_service", loadedModules(session_bus))

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
