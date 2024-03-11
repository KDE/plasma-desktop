#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

# pylint: disable=too-many-arguments

# For FreeBSD CI which only has python 3.9
from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from typing import Final

import numpy as np

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import cv2 as cv
from desktoptest import name_has_owner, start_plasmashell
from gi.repository import Gio, GLib

KDE_VERSION: Final = 6
PLASMASHELL_SERVICE_NAME: Final = "org.kde.plasmashell"


class GLibMainLoopThread(threading.Thread):
    """
    A GLib main loop in another thread.
    """

    def __init__(self) -> None:
        # Set up D-Bus loop
        self.loop = GLib.MainLoop()
        self.failSafeTimer = threading.Timer(600, self.loop.quit)

        # Create the thread
        super().__init__()

    def run(self) -> None:
        """
        Method to run the DBus main loop (on a thread)
        """
        self.failSafeTimer.start()
        self.loop.run()

    def quit(self) -> None:
        self.failSafeTimer.cancel()
        self.loop.quit()


class OrgKdeKSplash:
    """
    D-Bus interfaces for org.kde.KSplash
    """

    BUS_NAME: Final = "org.kde.KSplash"
    OBJECT_PATH: Final = "/KSplash"

    connection: Gio.DBusConnection

    def __init__(self) -> None:
        self.reg_id: int = 0
        self.registered_event = threading.Event()
        self.owner_id: int = Gio.bus_own_name(Gio.BusType.SESSION, self.BUS_NAME, Gio.BusNameOwnerFlags.NONE, self.on_bus_acquired, None, None)
        assert self.owner_id > 0

        self.stage: str = ""
        self.stage_set_event = threading.Event()

    def quit(self) -> None:
        self.connection.unregister_object(self.reg_id)
        Gio.bus_unown_name(self.owner_id)
        self.connection.flush_sync(None)

    def on_bus_acquired(self, connection: Gio.DBusConnection, name: str, *args) -> None:
        """
        Interface is ready, now register objects.
        """
        self.connection = connection

        introspection_data = Gio.DBusNodeInfo.new_for_xml("""
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
    "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
  <interface name="org.kde.KSplash">
    <method name="setStage">
      <arg name="stage" type="s" direction="in"/>
    </method>
  </interface>
</node>
""")
        self.reg_id = connection.register_object(self.OBJECT_PATH, introspection_data.interfaces[0], self.handle_method_call, None, None)
        assert self.reg_id > 0

        self.registered_event.set()

    def handle_method_call(self, connection: Gio.DBusConnection, sender: str, object_path: str, interface_name: str, method_name: str, parameters: GLib.Variant, invocation: Gio.DBusMethodInvocation) -> None:
        """
        Handles method calls for org.kde.KSplash
        """
        assert interface_name == self.BUS_NAME, f"Unknown interface {interface_name}"
        print(f"ksplash call {method_name}", file=sys.stderr, flush=True)

        if method_name == "setStage":
            if not parameters.is_of_type(GLib.VariantType("(s)")):
                invocation.return_error_literal(Gio.dbus_error_quark(), Gio.DBusError.INVALID_ARGS, f"Incorrect signature {parameters.get_type_string()}")
                return
            self.stage = parameters.unpack()[0]
            self.stage_set_event.set()
            invocation.return_value(None)
        else:
            invocation.return_error_literal(Gio.dbus_error_quark(), Gio.DBusError.UNKNOWN_METHOD, f"Unknown method {method_name}")


class Bug482267Test(unittest.TestCase):
    """
    Startup slow with Picture of the day
    """

    temp_home: tempfile.TemporaryDirectory

    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    session_bus: Gio.DBusConnection

    @classmethod
    def setUpClass(cls) -> None:
        # create a throw-away XDG home, so the test starts with a clean slate
        # with every run, and doesn't mess with your local installation
        cls.temp_home = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp_home.cleanup)
        os.environ["XDG_CACHE_HOME"] = os.path.join(cls.temp_home.name, ".cache")
        os.makedirs(os.environ["XDG_CACHE_HOME"])
        os.environ["XDG_CONFIG_HOME"] = os.path.join(cls.temp_home.name, ".config")
        os.makedirs(os.environ["XDG_CONFIG_HOME"])
        os.environ["XDG_DATA_HOME"] = os.path.join(cls.temp_home.name, ".local", "share")
        os.makedirs(os.environ["XDG_DATA_HOME"])
        os.environ["XDG_STATE_HOME"] = os.path.join(cls.temp_home.name, ".local", "state")
        os.makedirs(os.environ["XDG_STATE_HOME"])

        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()
        cls.session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)

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

    def test_1_set_color_wallpaper(self) -> None:
        """
        The color wallpaper doesn't set WallpaperItem.loading on construction, which is ideal for this test.
        """
        assert self.session_bus is not None and self.plasmashell is not None
        message: Gio.DBusMessage = Gio.DBusMessage.new_method_call(PLASMASHELL_SERVICE_NAME, "/PlasmaShell", "org.kde.PlasmaShell", "setWallpaper")
        params: dict[str, GLib.Variant] = {
            "Color": GLib.Variant("(u)", [4294901760]),  # RGBA value
        }
        # wallpaperPlugin(s), parameters(a{sv}), screenNum(u)
        message.set_body(GLib.Variant("(sa{sv}u)", ["org.kde.color", params, 0]))
        reply, _ = self.session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 10000)
        self.assertEqual(reply.get_message_type(), Gio.DBusMessageType.METHOD_RETURN)

        time.sleep(3)  # Desktop animation

        # Match the red area in the screenshot
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path: str = os.path.join(temp_dir, "desktop_screenshot.png")
            subprocess.check_call(["import", "-window", "root", saved_image_path])
            cv_image1 = cv.imread(saved_image_path, cv.IMREAD_COLOR)

            # Create a red square
            cv_image2 = np.zeros((100, 100, 3), dtype=np.uint8)
            cv_image2[:, :] = [0, 0, 255]

            matched = cv.matchTemplate(cv_image1, cv_image2, cv.TM_SQDIFF_NORMED)
            min_val, max_val, min_loc, max_loc = cv.minMaxLoc(matched)
            self.assertAlmostEqual(min_val, 0.0)

        # Prepare to restart plasmashell
        subprocess.check_output([f"kquitapp{KDE_VERSION}", "plasmashell"], stderr=sys.stderr)
        self.assertEqual(self.plasmashell.wait(5), 0)
        self.plasmashell = None

    def test_2_signal_ksplash_when_desktop_is_ready(self):
        """
        Make sure ShellCorona gives the ready signal to KSplash when wallpaper is loaded
        @see https://invent.kde.org/plasma/libplasma/-/commit/1b154b3eb2b7617067dfd05d961e53238458714e
        """
        loop_thread = GLibMainLoopThread()
        loop_thread.start()
        ksplash_interface = OrgKdeKSplash()
        self.addCleanup(loop_thread.quit)
        self.addCleanup(ksplash_interface.quit)
        self.assertTrue(ksplash_interface.registered_event.wait(5))
        self.assertEqual(ksplash_interface.stage, "")

        _, _, self.plasmashell = start_plasmashell()
        self.assertTrue(ksplash_interface.stage_set_event.wait(30))
        self.assertEqual(ksplash_interface.stage, "desktop")  # ShellCorona::checkAllDesktopsUiReady


if __name__ == '__main__':
    if "KDECI_BUILD" not in os.environ:
        sys.exit(0)
    unittest.main()
