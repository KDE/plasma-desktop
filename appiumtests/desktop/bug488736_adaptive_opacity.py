#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

# pylint: disable=too-many-arguments

# For FreeBSD CI which only has python 3.9
from __future__ import annotations

import base64
import os
import subprocess
import sys
import tempfile
import time
import unittest
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_plasmashell
from gi.repository import Gio, GLib

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from desktoptest import name_has_owner, start_plasmashell

gi.require_version('Gdk', '4.0')
gi.require_version('GdkPixbuf', '2.0')
gi.require_version('Gtk', '4.0')
from gi.repository import Gdk, GdkPixbuf, Gio, GLib, Gtk

KDE_VERSION: Final = 6
PLASMASHELL_SERVICE_NAME: Final = "org.kde.plasmashell"


class TestWindow(Gtk.ApplicationWindow):

    def __init__(self, _app: Gtk.Application) -> None:
        super().__init__(application=_app, title="Bug488736Test")
        self.set_default_size(10, 10)
        self.maximize()
        GLib.timeout_add_seconds(60, _app.quit)


class Bug488736Test(unittest.TestCase):
    """
    Adaptive opacity doesn't work when the panel is not floating
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    session_bus: Gio.DBusConnection

    @classmethod
    def setUpClass(cls) -> None:
        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()
        cls.session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)
        assert cls.plasmashell is not None

        options = AppiumOptions()
        options.set_capability("app", "Root")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_bug488736_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.plasmashell is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", "plasmashell"], stderr=sys.stderr)
            cls.plasmashell.wait(5)
        if cls.kded is not None:
            cls.kded.terminate()
            cls.kded.wait(5)
        if cls.kactivitymanagerd is not None:
            cls.kactivitymanagerd.terminate()
            cls.kactivitymanagerd.wait(5)

    def test_0_wait_until_ready(self) -> None:
        """
        Start plasmashell and wait until the DBus interface is ready
        """
        if not name_has_owner(self.session_bus, PLASMASHELL_SERVICE_NAME):
            for _ in range(10):
                if name_has_owner(self.session_bus, PLASMASHELL_SERVICE_NAME):
                    return
                print("waiting for plasmashell to appear on the DBus session")
                time.sleep(1)
            self.fail("plasmashell does not appear on DBus after 10 secs")
        self.driver.find_element(AppiumBy.NAME, "Application Launcher")

    def test_1_defloat_panel_and_set_color_background(self) -> None:
        """
        Defloat all panels to test the bug
        """
        assert self.session_bus is not None and self.plasmashell is not None
        message = Gio.DBusMessage.new_method_call(PLASMASHELL_SERVICE_NAME, "/PlasmaShell", "org.kde.PlasmaShell", "setWallpaper")
        params: dict[str, GLib.Variant] = {
            "Color": GLib.Variant("(u)", [4294901760]),  # RGBA value
        }
        # wallpaperPlugin(s), parameters(a{sv}), screenNum(u)
        message.set_body(GLib.Variant("(sa{sv}u)", ["org.kde.color", params, 0]))
        reply, _ = self.session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 100000)
        self.assertEqual(reply.get_message_type(), Gio.DBusMessageType.METHOD_RETURN)

        message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript")
        message.set_body(GLib.Variant("(s)", ["panels().forEach(containment => containment.floating = false)"]))
        reply, _ = self.session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 10000)
        self.assertEqual(reply.get_message_type(), Gio.DBusMessageType.METHOD_RETURN)

    def take_screenshot(self) -> str:
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path = os.path.join(temp_dir, "desktop.png")
            self.driver.get_screenshot_as_file(saved_image_path)
            return base64.b64encode(Gdk.Texture.new_from_filename(saved_image_path).save_to_png_bytes().get_data()).decode()

    def test_2_maximized_window(self):
        """
        When a window is maximized in the foreground, the panel should be translucent.
        """
        maximized_window = subprocess.Popen([os.path.abspath(__file__), "1"], stdout=sys.stderr, stderr=sys.stderr)
        self.addCleanup(maximized_window.kill)
        time.sleep(3)  # Window animation
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path: str = os.path.join(temp_dir, "desktop_screenshot.png")
            self.driver.get_screenshot_as_file(saved_image_path)
            pixbuf1 = GdkPixbuf.Pixbuf.new_from_file(saved_image_path)

        maximized_window.kill()
        time.sleep(3)  # Window animation
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path: str = os.path.join(temp_dir, "desktop_screenshot.png")
            self.driver.get_screenshot_as_file(saved_image_path)
            pixbuf2 = GdkPixbuf.Pixbuf.new_from_file(saved_image_path)

        self.assertEqual(pixbuf1.get_width(), pixbuf2.get_width())
        self.assertEqual(pixbuf1.get_height(), pixbuf2.get_height())
        self.assertGreater(pixbuf1.get_height(), 0)

        n_channels: int = pixbuf1.get_n_channels()

        pixel1 = pixbuf1.read_pixel_bytes().get_data()
        pixel2 = pixbuf2.read_pixel_bytes().get_data()
        # Get the pixel at the same position, 10 is a reasonable number to offset from the bottom line
        offset = (pixbuf1.get_height() - 10) * pixbuf1.get_width() * n_channels + int(pixbuf1.get_width() / 2) * n_channels
        self.assertNotEqual((pixel1[offset], pixel1[offset + 1], pixel1[offset + 2]), (pixel2[offset], pixel2[offset + 1], pixel2[offset + 2]))


def on_activate(_app: Gtk.Application) -> None:
    win = TestWindow(_app)
    win.set_visible(True)


if __name__ == '__main__':
    if len(sys.argv) == 1:
        unittest.main()
    else:
        sys.argv.pop()
        app = Gtk.Application(application_id='org.kde.testwindow')
        app.connect('activate', on_activate)
        app.run(None)
