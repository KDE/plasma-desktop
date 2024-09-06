#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import gi

gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class TestWindow(Gtk.Window):

    def __init__(self) -> None:
        super().__init__(title="Test Window")

        self.button = Gtk.Button.new_with_label(label="Active Window")
        self.button.connect("clicked", self.on_button_clicked)
        self.add(self.button)
        self.connect("notify::is-active", self.on_is_active_changed)

        GLib.timeout_add_seconds(60, self.close)

    def on_button_clicked(self, widget) -> None:
        self.close()

    def on_is_active_changed(self, instance, param) -> None:
        if self.is_active():
            self.button.set_label("Active Window")
        else:
            self.button.set_label("Inactive Window")


if __name__ == "__main__":
    win = TestWindow()
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()
