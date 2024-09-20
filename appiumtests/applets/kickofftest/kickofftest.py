#!/usr/bin/env python3

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

import os
import shutil
import subprocess
import sys
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from gi.repository import GLib
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir, "desktop"))
from desktoptest import start_kactivitymanagerd

WIDGET_ID: Final = "org.kde.plasma.kickoff"
KDE_VERSION: Final = 6


class KickoffTests(unittest.TestCase):
    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        os.makedirs(os.path.join(GLib.get_user_config_dir(), "menus"))
        shutil.copy(os.path.join(os.path.dirname(os.path.abspath(__file__)), "applications.menu"), os.path.join(GLib.get_user_config_dir(), "menus"))
        cls.kactivitymanagerd = start_kactivitymanagerd()
        subprocess.check_call([f"kbuildsycoca{KDE_VERSION}"], stdout=sys.stderr, stderr=sys.stderr)

        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
        })
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    @classmethod
    def tearDownClass(self) -> None:
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "plasmawindowed"])
        self.driver.quit()
        if self.kactivitymanagerd is not None:
            self.kactivitymanagerd.terminate()
            self.kactivitymanagerd.wait()

    def setUp(self) -> None:
        ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | All Applications]")

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{WIDGET_ID}_#{self.id()}.png")

    def test_1_search_calculator(self) -> None:
        self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("12345+67890")
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | 80235]")

    def test_2_search_app(self) -> None:
        # Emoji Selector is the only actual application we install from workspace :|
        self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("Emoji Selector")
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | Emoji Selector]").click()
        time.sleep(1)
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "plasma.emojier"])

    def test_3_keyboard_navigation(self) -> None:
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//table_cell[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        first_favorite = focused_elements[0].id
        self.assertIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))

        # Go right to second favorite
        ActionChains(self.driver).send_keys(Keys.RIGHT).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//table_cell[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)

        # Go left to first favorite again
        ActionChains(self.driver).send_keys(Keys.LEFT).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//table_cell[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertEqual(first_favorite, focused_elements[0].id)

        # Go further left to the category list
        ActionChains(self.driver).key_down(Keys.SHIFT).send_keys(Keys.TAB).key_up(Keys.ALT).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        favorites_category = focused_elements[0].id

        # Go down to the 'all apps' category
        ActionChains(self.driver).send_keys(Keys.DOWN).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)

        # Go right to the first all app. This must not be a grid delegate anymore (favorites are griddelegates)
        ActionChains(self.driver).send_keys(Keys.RIGHT).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        self.assertNotIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))
        first_all_app = focused_elements[0].id

        # Go down to second all app
        ActionChains(self.driver).send_keys(Keys.DOWN).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_all_app, focused_elements[0].id)

        # Hitting Tab should get us to the "menu bar" at the bottom
        ActionChains(self.driver).send_keys(Keys.TAB).perform()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//page_tab[contains(@states, 'focused') and @name='Applications']")
        self.assertEqual(len(focused_elements), 1)

        # Hitting Right should get us to the "Places" page
        ActionChains(self.driver).send_keys(Keys.RIGHT).perform()
        self.driver.find_element(AppiumBy.NAME, "Frequently Used")

        # Hitting Right again should move focus to the power action button
        ActionChains(self.driver).send_keys(Keys.RIGHT).perform()
        self.driver.find_element(by=AppiumBy.XPATH, value="//push_button_menu[contains(@states, 'focused') and @name='Leave']")


if __name__ == '__main__':
    unittest.main()
