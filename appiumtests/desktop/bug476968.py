#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import subprocess
import sys
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_kactivitymanagerd, start_kded
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait


class Bug476968Test(unittest.TestCase):
    """
    plasmashell crashes when clicking configure button in System Tray settings window for applets without an overridden configure action
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver and the desktop folder
        """
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

    def test_bug476968(self) -> None:
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Show hidden icons"))).click()  # plasma-workspace/applets/systemtray/package/contents/ui/ExpanderArrow.qml
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Configure System Tray..."))).click()
        entries = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Entries")))
        entries.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Configure Camera Indicator..."))).click()
        wait.until_not(lambda _: entries.is_displayed())  # Wait until the old window closes
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Camera Indicator Settings")))  # Window title


if __name__ == '__main__':
    unittest.main()
