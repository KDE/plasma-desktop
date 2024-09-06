#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import gi

gi.require_version('Gtk', '3.0')
from gi.repository import GLib, Gtk


class TestWindow(Gtk.Window):

    def __init__(self) -> None:
        super().__init__(title="Test Window")

        button = Gtk.Button.new_with_label("Install Software")
        button.connect("clicked", self.on_button_clicked)
        self.add(button)

        GLib.timeout_add_seconds(120, self.close)

    def on_button_clicked(self, widget) -> None:
        self.close()


if __name__ == "__main__":
    win = TestWindow()
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()
