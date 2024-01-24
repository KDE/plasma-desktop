#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import base64
import os
import subprocess
import sys
import tempfile
import time
import unittest

import cv2 as cv
import gi

gi.require_version('Gtk', '4.0')
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_kactivitymanagerd, start_kded
from gi.repository import Gtk
from selenium.common.exceptions import TimeoutException
from selenium.webdriver.common.actions.action_builder import ActionBuilder
from selenium.webdriver.common.actions.interaction import POINTER_TOUCH
from selenium.webdriver.common.actions.pointer_input import PointerInput
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait


class Bug477220Test(unittest.TestCase):

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        cls.kactivitymanagerd = start_kactivitymanagerd()
        cls.kded = start_kded()

        options = AppiumOptions()
        options.set_capability("app", "plasmashell -p org.kde.plasma.desktop --no-respawn")
        options.set_capability("environ", {
            "QT_LOGGING_RULES": "kf.kirigami.platform.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_bug476968_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_output(["kquitapp6", "plasmashell"], stderr=sys.stderr)
        if cls.kded:
            cls.kded.kill()
        if cls.kactivitymanagerd:
            cls.kactivitymanagerd.kill()
        cls.driver.quit()

    def test_1_bug478958_touch_long_press_on_desktop(self) -> None:
        """
        Long press on the desktop to enter the edit mode
        """
        time.sleep(3)
        screen_geometry = Gtk.Window().get_display().get_monitors()[0].get_geometry()
        long_press_time_ms: int = Gtk.Settings.get_default().get_property("gtk-long-press-time") * 2 + 5000
        self.assertGreater(screen_geometry.width, 100)
        self.assertGreater(screen_geometry.height, 100)

        # Click "More" to open the desktop context menu
        wait = WebDriverWait(self.driver, 5)
        success = False
        for _ in range(20):
            try:
                # Work around "no target window"
                action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
                action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height / 2)).click()
                action.perform()
                action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
                action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height / 2)).pointer_down().pause(long_press_time_ms / 1000).pointer_up()
                action.perform()
                wait.until(EC.presence_of_element_located((AppiumBy.NAME, "More"))).click()
                success = True
                break
            except TimeoutException:
                continue
        self.assertTrue(success)

    @unittest.expectedFailure  # get_screenshot_as_file is broken in the CI
    def test_2_bug477220_open_context_menu(self) -> None:
        """
        "More" button in the desktop toolbox does not open the context menu
        """
        time.sleep(3)  # Wait until the menu appears
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path: str = os.path.join(temp_dir, "desktop.png")
            self.assertTrue(self.driver.get_screenshot_as_file(saved_image_path))

            cv_first_image = cv.imread(saved_image_path, cv.IMREAD_COLOR)
            first_image = base64.b64encode(cv.imencode('.png', cv_first_image)[1].tobytes()).decode()

        cv_expected_image = cv.imread(os.path.join(os.path.dirname(os.path.abspath(__file__)), "resources", "bug478958_new_file_menu_item_icon.png"), cv.IMREAD_COLOR)
        expected_image = base64.b64encode(cv.imencode('.png', cv_expected_image)[1].tobytes()).decode()
        self.driver.find_image_occurrence(first_image, expected_image, threshold=1e-7)

        # Click an empty area to close the menu
        screen_geometry = Gtk.Window().get_display().get_monitors()[0].get_geometry()
        action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
        action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height / 2)).click()
        action.perform()


if __name__ == '__main__':
    unittest.main()
