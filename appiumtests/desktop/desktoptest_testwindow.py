#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import gi

gi.require_version('Gtk', '4.0')
from gi.repository import GLib, Gtk


class TestWindow(Gtk.ApplicationWindow):

    def __init__(self, _app: Gtk.Application) -> None:
        super().__init__(application=_app, title="Test Window")

        self.button = Gtk.Button(label="Install Software")
        self.button.connect("clicked", self.on_button_clicked)
        self.set_child(self.button)

        GLib.timeout_add_seconds(120, self.close)

    def on_button_clicked(self, widget) -> None:
        self.close()


def on_activate(_app: Gtk.Application) -> None:
    win = TestWindow(_app)
    win.present()


if __name__ == "__main__":
    app = Gtk.Application(application_id='org.kde.testwindow')
    app.connect('activate', on_activate)
    app.run(None)
