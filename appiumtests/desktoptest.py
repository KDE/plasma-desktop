#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import pathlib
import subprocess
import sys
import sysconfig
import time
import unittest
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.support.ui import WebDriverWait

gi.require_version('Gdk', '3.0')
from gi.repository import Gdk, Gio, GLib

if "KDECI_BUILD" not in os.environ:
    CMAKE_INSTALL_PREFIX: Final = os.environ.get("CMAKE_INSTALL_PREFIX", os.path.join(pathlib.Path.home(), "kde", "usr"))
    SITE_PACKAGES_DIR: Final = os.path.join(CMAKE_INSTALL_PREFIX, sysconfig.get_path("platlib")[len(sys.prefix + os.sep):])
    for subdir in os.listdir(SITE_PACKAGES_DIR):
        sys.path.append(os.path.join(SITE_PACKAGES_DIR, subdir))
import inputsynth_plasma_desktop as IS

CMAKE_BINARY_DIR: Final = os.environ.get("CMAKE_BINARY_DIR", os.path.join(pathlib.Path.home(), "kde/build/plasma-desktop/bin"))
KACTIVITYMANAGERD_PATH: Final = os.environ.get("KACTIVITYMANAGERD_PATH", os.path.join(pathlib.Path.home(), "kde/usr/lib64/libexec/kactivitymanagerd"))
KACTIVITYMANAGERD_SERVICE_NAME: Final = "org.kde.ActivityManager"
EVDEV_OFFSET: Final = 8


def name_has_owner(session_bus: Gio.DBusConnection, name: str) -> bool:
    """
    Whether the given name is available on session bus
    """
    message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner")
    message.set_body(GLib.Variant("(s)", [name]))
    reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
    return reply and reply.get_signature() == 'b' and reply.get_body().get_child_value(0).get_boolean()


def keyval_to_keycode(key_val: int) -> int:
    """
    @param key_val see https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h
    """
    match key_val:
        case 0xffe1:  # XK_Shift_L
            return 42 + EVDEV_OFFSET
        case 0xffe9:  #XK_Alt_L
            return 56 + EVDEV_OFFSET
        case 0xffe3:  # XK_Control_L
            return 29 + EVDEV_OFFSET

    keymap = Gdk.Keymap.get_default()
    ret, keys = keymap.get_entries_for_keyval(key_val)
    if not ret:
        raise RuntimeError("Failed to map key!")
    return keys[0].keycode


class DesktopTest(unittest.TestCase):
    """
    Tests for the desktop package
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver
        """
        cls.kactivitymanagerd = subprocess.Popen([KACTIVITYMANAGERD_PATH], stdout=sys.stderr, stderr=sys.stderr)
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        kactivitymanagerd_started: bool = False
        for _ in range(10):
            if name_has_owner(session_bus, KACTIVITYMANAGERD_SERVICE_NAME):
                kactivitymanagerd_started = True
                break
            print("waiting for kactivitymanagerd to appear on the DBus session")
            time.sleep(1)
        assert kactivitymanagerd_started

        IS.init_module()

        options = AppiumOptions()
        options.set_capability("app", "plasmashell -p org.kde.plasma.desktop --no-respawn")
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
        subprocess.check_output(["kquitapp6", "plasmashell"], stderr=sys.stderr)
        cls.kactivitymanagerd.kill()
        cls.driver.quit()

    def test_0_panel_ready(self) -> None:
        """
        Until the panel is ready
        """
        self.driver.find_element(AppiumBy.NAME, "Application Launcher")

    def test_1_open_edit_mode(self) -> None:
        """
        Tests the edit mode toolbox can be loaded
        Consolidates https://invent.kde.org/frameworks/plasma-framework/-/commit/3bb099a427cacd44fef7668225d8094f952dd5b2
        """
        # Key values are from https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h
        # Alt+D
        IS.key_press(keyval_to_keycode(0xffe9))
        IS.key_press(keyval_to_keycode(0x0044))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(0x0044))
        IS.key_release(keyval_to_keycode(0xffe9))
        time.sleep(0.5)
        # E
        IS.key_press(keyval_to_keycode(0x0045))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(0x0045))

        global_theme_button = self.driver.find_element(AppiumBy.NAME, "Choose Global Theme…")
        self.driver.find_element(AppiumBy.NAME, "Exit Edit Mode").click()
        WebDriverWait(self.driver, 5).until(lambda _: not global_theme_button.is_displayed())


if __name__ == '__main__':
    unittest.main()
