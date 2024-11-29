#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>
# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import functools
import logging
import os
import pathlib
import shutil
import stat
import subprocess
import sys
import time
import unittest
from typing import Final

import cv2 as cv
import gi

gi.require_version('Gtk', '4.0')
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib, Gtk
from selenium.common.exceptions import TimeoutException
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.actions.action_builder import ActionBuilder
from selenium.webdriver.common.actions.interaction import POINTER_TOUCH
from selenium.webdriver.common.actions.pointer_input import PointerInput
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

KACTIVITYMANAGERD_PATH: Final = os.environ.get("KACTIVITYMANAGERD_PATH", os.path.join(pathlib.Path.home(), "kde/usr/lib64/libexec/kactivitymanagerd"))
KACTIVITYMANAGERD_SERVICE_NAME: Final = "org.kde.ActivityManager"
KDE_VERSION: Final = 6


def name_has_owner(session_bus: Gio.DBusConnection | None, name: str) -> bool:
    """
    Whether the given name is available on session bus
    """
    if session_bus is None:
        session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)
    message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "NameHasOwner")
    message.set_body(GLib.Variant("(s)", [name]))
    reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
    return reply and reply.get_signature() == 'b' and reply.get_body().get_child_value(0).get_boolean()


def build_ksycoca() -> None:
    subprocess.check_call([f"kbuildsycoca{KDE_VERSION}"], stdout=sys.stderr, stderr=sys.stderr, env=os.environ)


def start_kactivitymanagerd() -> subprocess.Popen | None:
    session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
    if name_has_owner(session_bus, KACTIVITYMANAGERD_SERVICE_NAME):
        return None

    os.makedirs(os.path.join(GLib.get_user_config_dir(), "menus"))
    shutil.copy(os.path.join(os.path.dirname(os.path.abspath(__file__)), "applications.menu"), os.path.join(GLib.get_user_config_dir(), "menus"))

    kactivitymanagerd = subprocess.Popen([KACTIVITYMANAGERD_PATH], stdout=sys.stderr, stderr=sys.stderr, env=os.environ)
    started: bool = False
    for _ in range(10):
        if name_has_owner(session_bus, KACTIVITYMANAGERD_SERVICE_NAME):
            started = True
            break
        logging.info("waiting for kactivitymanagerd to appear on the DBus session")
        time.sleep(1)
    assert started

    build_ksycoca()

    return kactivitymanagerd


def start_kded() -> subprocess.Popen | None:
    session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
    kded = None
    if not name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
        kded = subprocess.Popen([f"kded{KDE_VERSION}"], stdout=sys.stderr, stderr=sys.stderr)
        kded_started: bool = False
        for _ in range(10):
            if name_has_owner(session_bus, f"org.kde.kded{KDE_VERSION}"):
                kded_started = True
                break
            print(f"waiting for kded{KDE_VERSION} to appear on the dbus session")
            time.sleep(1)
        assert kded_started

    return kded


def start_plasmashell() -> tuple:
    """
    Launches plashashell and returns the subprocess instances
    """
    kactivitymanagerd = start_kactivitymanagerd()
    kded = start_kded()
    eng_env = os.environ.copy()
    eng_env["LC_ALL"] = "en_US.UTF-8"
    plasmashell = subprocess.Popen(["plasmashell", "-p", "org.kde.plasma.desktop", "--no-respawn"], stdout=sys.stderr, stderr=sys.stderr, env=eng_env)

    return (kactivitymanagerd, kded, plasmashell)


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
        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()
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
        if cls.plasmashell is not None:
            subprocess.check_output(["kquitapp6", "plasmashell"], stderr=sys.stderr)
            assert cls.plasmashell.wait(30) == 0, cls.plasmashell.returncode

        if cls.kded:
            cls.kded.kill()
        if cls.kactivitymanagerd:
            cls.kactivitymanagerd.kill()
        cls.driver.quit()

    def _exit_edit_mode(self) -> None:
        """
        Finds the close button and clicks it
        """
        global_theme_button = self.driver.find_element(AppiumBy.NAME, "Global Themes")
        self.driver.find_element(AppiumBy.NAME, "Exit Edit Mode").click()
        WebDriverWait(self.driver, 30).until_not(lambda _: global_theme_button.is_displayed())

    def _open_containment_config_dialog(self) -> None:
        # Alt+D, S
        ActionChains(self.driver).key_down(Keys.ALT).send_keys("d").pause(0.5).send_keys("s").key_up(Keys.ALT).perform()
        WebDriverWait(self.driver, 30).until(EC.presence_of_element_located((AppiumBy.NAME, "Wallpaper type:")))

    def test_0_panel_ready(self) -> None:
        """
        Waits until the panel is ready
        """
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Application Launcher")))

    def test_1_containment_config_dialog_2_add_new_wallpaper(self) -> None:
        """
        Tests if the file dialog is opened successfully
        @see https://invent.kde.org/plasma/plasma-integration/-/merge_requests/117
        """
        self._open_containment_config_dialog()
        self.driver.find_element(AppiumBy.NAME, "Add Wallpaper Image…").click()
        wait = WebDriverWait(self.driver, 30)
        title_element: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Open Image")))
        buttons = self.driver.find_elements(AppiumBy.XPATH, "//button[@name='Cancel' and contains(@accessibility-id, 'KFileWidget')]")
        for button in buttons:
            button.click()
        wait.until_not(lambda _: title_element.is_displayed())

    def test_1_containment_config_dialog_3_other_sections(self) -> None:
        """
        Opens other sections successively and matches text to make sure there is no breaking QML error
        """
        self._open_containment_config_dialog()
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

        ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
        wait.until_not(lambda _: mouseaction_element.is_displayed())

    def test_3_open_panel_edit_mode(self) -> None:
        """
        Tests the edit mode toolbox can be loaded
        Consolidates https://invent.kde.org/frameworks/plasma-framework/-/commit/3bb099a427cacd44fef7668225d8094f952dd5b2
        """
        # Alt+D, E
        actions = ActionChains(self.driver)
        actions.key_down(Keys.ALT).send_keys("d").key_up(Keys.ALT).perform()
        actions.send_keys("e").perform()

        wait = WebDriverWait(self.driver, 30)
        self.driver.find_element(AppiumBy.NAME, "Configure Panel…").click()
        widget_button: WebElement = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Widgets…")))
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Spacer")))

        actions.send_keys(Keys.ESCAPE).perform()
        wait.until_not(lambda _: widget_button.is_displayed())

        self._exit_edit_mode()

    def test_5_bug477185_meta_number_shortcut(self) -> None:
        """
        Meta+1 should activate the first launcher item
        """
        # Prepare a desktop file
        os.makedirs(os.path.join(GLib.get_user_data_dir(), "applications"))
        desktopfile_path = os.path.join(GLib.get_user_data_dir(), "applications", "org.kde.testwindow.desktop")
        with open(desktopfile_path, "w", encoding="utf-8") as file_handler:
            file_handler.writelines([
                "[Desktop Entry]\n",
                "Type=Application\n",
                "Icon=preferences-system\n",
                "Name=Software Center\n",
                f"Exec=python3 {os.path.join(os.path.dirname(os.path.abspath(__file__)), 'desktoptest_testwindow.py')}\n",
            ])
            file_handler.flush()
        os.chmod(desktopfile_path, os.stat(desktopfile_path).st_mode | stat.S_IEXEC)
        self.addCleanup(functools.partial(os.remove, desktopfile_path))

        session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)

        # Add a new launcher item
        message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "evaluateScript")
        message.set_body(GLib.Variant("(s)", [f"panels().forEach(containment => containment.widgets('org.kde.plasma.icontasks').forEach(widget => {{widget.currentConfigGroup = ['General'];widget.writeConfig('launchers', 'file://{desktopfile_path}');}}))"]))
        session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)

        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Software Center")))

        # Activate the first launcher
        # ActionChains(self.driver).key_down(Keys.META).send_keys("1").key_up(Keys.META) # FIXME Meta modifier doesn't work
        message = Gio.DBusMessage.new_method_call("org.kde.kglobalaccel", "/component/plasmashell", "org.kde.kglobalaccel.Component", "invokeShortcut")
        message.set_body(GLib.Variant("(s)", ["activate task manager entry 1"]))
        session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Install Software")))

    @unittest.skipIf("KDECI_BUILD" in os.environ, "TODO: failed in CI")
    def test_6_sentry_3516_load_layout(self) -> None:
        """
        ShellCorona::loadLookAndFeelDefaultLayout -> ShellCorona::unload() -> qDeleteAll(panelViews) -> QWindow::visibleChanged -> rectNotify() -> 💣
        @see https://bugreports.qt.io/browse/QTBUG-118841
        """
        kickoff_element = self.driver.find_element(AppiumBy.NAME, "Application Launcher")
        session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)
        # LookAndFeelManager::save
        message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "loadLookAndFeelDefaultLayout")
        message.set_body(GLib.Variant("(s)", ["org.kde.breezedark.desktop"]))
        session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 10000)
        self.assertFalse(kickoff_element.is_displayed())
        self.driver.find_element(AppiumBy.NAME, "Application Launcher")

    def test_7_bug_query_accent_color_binding_loop(self) -> None:
        """
        Don't use binding as usedInAccentColor may be disabled immediately after a query from kcm_colors.

        The call order is:
        1. desktop.usedInAccentColor: true -> wallpaperColors.active: true
        2. Kirigami.ImageColors.update()
        3. Kirigami.ImageColors emits paletteChanged()
        5. If binding is used, ShellCorona::colorChanged is emitted immediately in the same context after desktop.accentColor is updated
           -> desktop.usedInAccentColor: false -> desktop.accentColor: "transparent" (binding restoration).
        6. The second time querying the accent color, the QML engine will report binding loop detected in desktop.usedInAccentColor,
           and desktop.accentColor will return "transparent" directly before any accent color from the wallpaper is extracted.
        """
        session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)
        # Send the same request twice to check binding loop
        for i in range(2):
            print(f"sending message {i}", file=sys.stderr)
            message: Gio.DBusMessage = Gio.DBusMessage.new_method_call("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "color")
            reply, _ = session_bus.send_message_with_reply_sync(message, Gio.DBusSendMessageFlags.NONE, 1000)
            self.assertEqual(reply.get_signature(), "u")
            self.assertGreater(reply.get_body().get_child_value(0).get_uint32(), 0)
            print("done sending message", file=sys.stderr)

    def test_8_touch_long_press_on_desktop(self) -> None:
        # BUG478958: Long press on the desktop to enter the edit mode
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

        # BUG 477220: "More" button in the desktop toolbox does not open the context menu
        time.sleep(3)  # Wait until the menu appears
        self.driver.get_screenshot_as_file("bug477220_open_context_menu_before.png")
        actions = ActionChains(self.driver)
        for _ in range(3):  # The number of menu items by default
            actions = actions.send_keys(Keys.DOWN).pause(0.5)
        actions.perform()
        self.driver.get_screenshot_as_file("bug477220_open_context_menu_after.png")

        img1 = cv.imread("bug477220_open_context_menu_before.png", cv.IMREAD_GRAYSCALE)
        img2 = cv.imread("bug477220_open_context_menu_after.png", cv.IMREAD_GRAYSCALE)
        diff = cv.subtract(img1, img2)
        self.assertGreater(cv.countNonZero(diff), 4000)  # Menu highlight changes

        # Click an empty area to close the menu
        action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
        action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height / 2)).click()
        action.perform()

        # Long press on the panel to enter the edit mode
        time.sleep(3)  # Wait until the menu disappears
        wait = WebDriverWait(self.driver, 5)
        success = False
        for _ in range(20):
            try:
                # Work around "no target window"
                action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
                action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height - 20)).click()
                action.perform()
                # Press on the panel
                action = ActionBuilder(self.driver, mouse=PointerInput(POINTER_TOUCH, "finger"))
                action.pointer_action.move_to_location(int(screen_geometry.width / 2), int(screen_geometry.height - 20)).pointer_down().pause(long_press_time_ms / 1000).pointer_up()
                action.perform()
                wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Add Spacer")))  # The panel config dialog is opened
                success = True
                break
            except TimeoutException:
                continue
        self.assertTrue(success)

        self._exit_edit_mode()


if __name__ == '__main__':
    assert subprocess.call(["pidof", "plasmashell"]) != 0, "The test requires plasmashell to quit"
    unittest.main()
