#!/usr/bin/env python3

# SPDX-License-Identifier: MIT
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

import os
import subprocess
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

KDE_INSTALL_FULL_LIBEXECDIR = os.environ.get("KDE_INSTALL_FULL_LIBEXECDIR", "/usr/libexec")


class LogoutGreeterTests(unittest.TestCase):

    def setUp(self):
        self.proc = subprocess.Popen(["{}/ksmserver-logout-greeter".format(KDE_INSTALL_FULL_LIBEXECDIR), "--windowed"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        options = AppiumOptions()
        options.set_capability("app", str(self.proc.pid))
        options.set_capability("timeouts", {'implicit': 10000})
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
        })
        self.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self):
        self.driver.quit()
        self.assertEqual(self.proc.returncode != None, True)
        try:
            self.proc.terminate()
            self.proc.wait(10)
        except subprocess.TimeoutExpired:
            self.proc.kill()
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_logoutgreetertest_{self.id()}.png")

    def assertStdOutLine(self, expected):
        out, err = self.proc.communicate()
        outLines = out.splitlines()
        self.assertEqual(expected, outLines[(len(outLines) - 1)].decode('UTF-8'))

    def test_sleep(self):
        self.driver.find_element(AppiumBy.XPATH, "//button[contains(@name, 'Slee') and contains(@accessibility-id, 'LogoutButton')]").click()
        self.assertStdOutLine("suspend")

    def test_hibernate(self):
        self.driver.find_element(AppiumBy.XPATH, "//button[contains(@name, 'Hibernate') and contains(@accessibility-id, 'LogoutButton')]").click()
        self.assertStdOutLine("hibernate")

    def test_restart(self):
        self.driver.find_element(AppiumBy.XPATH, "//button[contains(@name, 'Restart') and contains(@accessibility-id, 'LogoutButton')]").click()
        self.assertStdOutLine("reboot")

    def test_shutdown(self):
        self.driver.find_element(AppiumBy.XPATH, "//button[contains(@name, 'Shut Down') and contains(@accessibility-id, 'LogoutButton')]").click()
        self.assertStdOutLine("shutdown")


if __name__ == '__main__':
    unittest.main()
