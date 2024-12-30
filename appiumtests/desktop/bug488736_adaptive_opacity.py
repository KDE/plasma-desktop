#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

# pylint: disable=too-many-arguments

import gi

gi.require_version('Gtk', '4.0')
from gi.repository import GLib, Gtk


class TestWindow(Gtk.ApplicationWindow):

    def __init__(self, _app: Gtk.Application) -> None:
        super().__init__(application=_app, title="Bug488736Test")
        self.set_default_size(10, 10)
        self.maximize()
        GLib.timeout_add_seconds(60, _app.quit)


def on_activate(_app: Gtk.Application) -> None:
    win = TestWindow(_app)
    win.set_visible(True)


if __name__ == '__main__':
    app = Gtk.Application(application_id='org.kde.testwindow')
    app.connect('activate', on_activate)
    app.run(None)
