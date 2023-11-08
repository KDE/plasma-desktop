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
from enum import Enum
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import ExtensionBase
from appium.webdriver.webelement import WebElement
from selenium.webdriver.support import expected_conditions as EC
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
KDE_VERSION: Final = 6


class XKeyCode(Enum):
    """
    @see https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h
    """
    Alt = 0xffe9
    Ctrl = 0xffe3
    D = 0x0044
    Down = 0xff54
    E = 0x0045
    Enter = 0xff8d
    Escape = 0xff1b
    Menu = 0xff67
    P = 0x0050
    Shift = 0xffe1
    Space = 0x0020
    Return = 0xff0d
    Right = 0xff53
    S = 0x0053
    Super = 0xffeb
    Tab = 0xff09
    Up = 0xff52


def name_has_owner(session_bus: Gio.DBusConnection, name: str) -> bool:
    """
    Whether the given name is available on session bus
    """
    message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner")
    message.set_body(GLib.Variant("(s)", [name]))
    reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
    return reply and reply.get_signature() == 'b' and reply.get_body().get_child_value(0).get_boolean()


def keyval_to_keycode(key_val: XKeyCode) -> int:
    """
    @param key_val see https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h
    """
    match key_val:
        case XKeyCode.Shift:  # XK_Shift_L
            return 42 + EVDEV_OFFSET
        case XKeyCode.Alt:  #XK_Alt_L
            return 56 + EVDEV_OFFSET
        case XKeyCode.Ctrl:  # XK_Control_L
            return 29 + EVDEV_OFFSET

    keymap = Gdk.Keymap.get_default()
    ret, keys = keymap.get_entries_for_keyval(key_val.value)
    if not ret:
        raise RuntimeError("Failed to map key!")
    return keys[0].keycode


class DesktopTest(unittest.TestCase):
    """
    Tests for the desktop package
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver
        """
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        if not name_has_owner(session_bus, KACTIVITYMANAGERD_SERVICE_NAME):
            cls.kactivitymanagerd = subprocess.Popen([KACTIVITYMANAGERD_PATH], stdout=sys.stderr, stderr=sys.stderr)
            kactivitymanagerd_started: bool = False
            for _ in range(10):
                if name_has_owner(session_bus, KACTIVITYMANAGERD_SERVICE_NAME):
                    kactivitymanagerd_started = True
                    break
                print("waiting for kactivitymanagerd to appear on the DBus session")
                time.sleep(1)
            assert kactivitymanagerd_started

        if not name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
            cls.kded = subprocess.Popen([f"kded{KDE_VERSION}"], stdout=sys.stderr, stderr=sys.stderr)
            kded_started: bool = False
            for _ in range(10):
                if name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
                    kded_started = True
                    break
                print(f"waiting for kded{KDE_VERSION} to appear on the dbus session")
                time.sleep(1)
            assert kded_started

        cls.plasmashell = subprocess.Popen(["plasmashell", "-p", "org.kde.plasma.desktop", "--no-respawn"], stdout=sys.stderr, stderr=sys.stderr)

        IS.init_module()

        options = AppiumOptions()
        options.set_capability("app", "Root")
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
        if cls.kded:
            cls.kded.kill()
        if cls.kactivitymanagerd:
            cls.kactivitymanagerd.kill()
        cls.driver.quit()

    def _enter_edit_mode(self) -> None:
        """
        Uses the keyboard shortcut to enter edit mode
        """
        # Key values are from https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h
        # Alt+D
        IS.key_press(keyval_to_keycode(XKeyCode.Alt))
        IS.key_press(keyval_to_keycode(XKeyCode.D))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(XKeyCode.Alt))
        IS.key_release(keyval_to_keycode(XKeyCode.D))
        time.sleep(0.5)
        # E
        IS.key_press(keyval_to_keycode(XKeyCode.E))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(XKeyCode.E))

    def _exit_edit_mode(self) -> None:
        """
        Finds the close button and clicks it
        """
        global_theme_button = self.driver.find_element(AppiumBy.NAME, "Choose Global Theme…")
        self.driver.find_element(AppiumBy.NAME, "Exit Edit Mode").click()
        WebDriverWait(self.driver, 30).until(lambda _: not global_theme_button.is_displayed())

    def test_0_panel_ready(self) -> None:
        """
        Waits until the panel is ready
        """
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Application Launcher")))

    def test_1_containment_config_dialog_1_open(self) -> None:
        """
        Opens the containment config dialog by clicking the button in the toolbox
        """
        # Alt+D
        IS.key_press(keyval_to_keycode(XKeyCode.Alt))
        IS.key_press(keyval_to_keycode(XKeyCode.D))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(XKeyCode.Alt))
        IS.key_release(keyval_to_keycode(XKeyCode.D))
        time.sleep(0.5)
        # S
        IS.key_press(keyval_to_keycode(XKeyCode.S))
        time.sleep(0.5)
        IS.key_release(keyval_to_keycode(XKeyCode.S))
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Wallpaper type:")))

    def test_1_containment_config_dialog_2_add_new_wallpaper(self) -> None:
        """
        Tests if the file dialog is opened successfully
        @see https://invent.kde.org/plasma/plasma-integration/-/merge_requests/117
        """
        self.driver.find_element(AppiumBy.NAME, "Add…").click()
        wait = WebDriverWait(self.driver, 30)
        title_element: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Open Image")))

        IS.key_press(keyval_to_keycode(XKeyCode.Escape))
        IS.key_release(keyval_to_keycode(XKeyCode.Escape))
        wait.until_not(lambda _: title_element.is_displayed())

    def test_1_containment_config_dialog_3_other_sections(self) -> None:
        """
        Opens other sections successively and matches text to make sure there is no breaking QML error
        """
        self.test_1_containment_config_dialog_1_open()

        wait = WebDriverWait(self.driver, 30)
        mouseaction_element: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Mouse Actions")))
        location_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Location")))
        icons_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Icons")))
        filter_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Filter")))

        mouseaction_element.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Action")))

        location_element.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Desktop folder")))

        icons_element.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Configure Preview Plugins…"))).click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Preview Plugins")))

        filter_element.click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "File name pattern:")))

        IS.key_press(keyval_to_keycode(XKeyCode.Escape))
        IS.key_release(keyval_to_keycode(XKeyCode.Escape))
        wait.until_not(lambda _: mouseaction_element.is_displayed())

    def test_3_open_panel_edit_mode(self) -> None:
        """
        Tests the edit mode toolbox can be loaded
        Consolidates https://invent.kde.org/frameworks/plasma-framework/-/commit/3bb099a427cacd44fef7668225d8094f952dd5b2
        """
        self._enter_edit_mode()

        wait = WebDriverWait(self.driver, 30)
        self.driver.find_element(AppiumBy.NAME, "Configure Panel…").click()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Widgets…")))
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Spacer")))

        self._exit_edit_mode()


if __name__ == '__main__':
    unittest.main()
