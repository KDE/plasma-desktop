#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import base64
import functools
import os
import subprocess
import sys
import tempfile
import time
import unittest

import cv2 as cv
import gi
import numpy as np
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_kactivitymanagerd, start_kded
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

gi.require_version('GdkPixbuf', '2.0')
from gi.repository import GdkPixbuf, GLib


class FolderViewTest(unittest.TestCase):
    """
    Tests for the desktop folder view layout
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    desktop_dir: str = ""

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver and the desktop folder
        """
        cls.desktop_dir = GLib.get_user_special_dir(GLib.UserDirectory.DIRECTORY_DESKTOP)
        if not os.path.exists(cls.desktop_dir):
            os.makedirs(cls.desktop_dir)
        # Create an image file
        bits_per_sample = 8
        width = height = 16
        pixbuf = GdkPixbuf.Pixbuf.new(GdkPixbuf.Colorspace.RGB, True, bits_per_sample, width, height)
        pixbuf.fill(0xff0000ff)
        assert pixbuf.savev(os.path.join(cls.desktop_dir, "test.png"), "png")
        cls.addClassCleanup(functools.partial(os.remove, os.path.join(cls.desktop_dir, "test.png")))

        cls.kactivitymanagerd = start_kactivitymanagerd()
        cls.kded = start_kded()

        options = AppiumOptions()
        options.set_capability("app", "plasmashell -p org.kde.plasma.desktop --no-respawn")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_folderview_#{self.id()}.png")

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

    def test_0_folderview_ready(self) -> None:
        """
        Waits until the folder view is ready
        """
        WebDriverWait(self.driver, 30).until(EC.presence_of_element_located((AppiumBy.NAME, "test.png")))
        time.sleep(3)  # Make sure the desktop is ready

    def test_1_image_preview(self) -> None:
        """
        test.png is filled with red color
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            # Take desktop screenshot
            saved_image_path: str = os.path.join(temp_dir, "desktop.png")
            subprocess.check_call(["import", "-window", "root", saved_image_path])
            cv_first_image = cv.imread(saved_image_path, cv.IMREAD_COLOR)
            first_image = base64.b64encode(cv.imencode('.png', cv_first_image)[1].tobytes()).decode()
        cv_second_image = np.zeros((16, 16, 3), dtype=np.uint8)
        cv_second_image[:, :] = [0, 0, 255]
        second_image = base64.b64encode(cv.imencode('.png', cv_second_image)[1].tobytes()).decode()
        self.driver.find_image_occurrence(first_image, second_image)

    def test_2_bug469260_new_file_appear_without_refresh(self) -> None:
        """
        A new file on desktop created by other applications should show up immediately
        """
        with open(os.path.join(self.desktop_dir, "test.txt"), "w", encoding="utf-8") as handler:
            handler.write("\n")

        self.addCleanup(functools.partial(os.remove, os.path.join(self.desktop_dir, "test.txt")))
        WebDriverWait(self.driver, 30).until(EC.presence_of_element_located((AppiumBy.NAME, "test.txt")))


if __name__ == '__main__':
    unittest.main()
