#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import gi

gi.require_version('Gtk', '4.0')
from gi.repository import GLib, Gtk


class TestWindow(Gtk.ApplicationWindow):

    def __init__(self, _app: Gtk.Application) -> None:
        super().__init__(application=_app, title="Test Window")

        self.button = Gtk.Button(label="Active Window")
        self.button.connect("clicked", self.on_button_clicked)
        self.set_child(self.button)
        self.connect("notify::is-active", self.on_is_active_changed)

        GLib.timeout_add_seconds(60, self.close)

    def on_button_clicked(self, widget) -> None:
        self.close()

    def on_is_active_changed(self, instance, param) -> None:
        if self.is_active():
            self.button.set_label("Active Window")
        else:
            self.button.set_label("Inactive Window")


def on_activate(_app: Gtk.Application) -> None:
    win = TestWindow(_app)
    win.present()


if __name__ == "__main__":
    app = Gtk.Application(application_id='org.kde.testwindow')
    app.connect('activate', on_activate)
    app.run(None)
