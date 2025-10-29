#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2025 David Edmundson <davidedmundson@kde.org>
# SPDX-License-Identifier: MIT

import logging
import os
import stat
import subprocess
import sys
from typing import Final, Self

from PySide6.QtCore import QStandardPaths, QTimer
from PySide6.QtGui import QImage, QColor
from PySide6.QtWidgets import QApplication, QMainWindow, QPushButton

KDE_VERSION: Final = 6


class DesktopFileWrapper(object):
    APPLICATION_ID: Final = "org.kde.testwindow"
    APPLICATION_NAME: Final = "Software Center"

    def __init__(self) -> None:
        data_dir = QStandardPaths.writableLocation(QStandardPaths.GenericDataLocation)  # usually ~/.local/share
        self.path: str = os.path.join(data_dir, "applications", f"{self.APPLICATION_ID}.desktop")
        self.application_path: str = os.path.realpath(__file__)
        self.icon_path: str = os.path.join(data_dir, "icons", "test.png")

    def create(self) -> None:
        data_dir = QStandardPaths.writableLocation(QStandardPaths.GenericDataLocation)
        os.makedirs(os.path.join(data_dir, "applications"), exist_ok=True)
        os.makedirs(os.path.join(data_dir, "icons"), exist_ok=True)

        # Create a 16x16 red RGBA icon
        img = QImage(16, 16, QImage.Format.Format_RGBA8888)
        img.fill(QColor(255, 0, 0, 255))  # solid red
        ok = img.save(self.icon_path, "PNG")
        assert ok, "Failed to save icon"

        with open(self.path, "w", encoding="utf-8") as fh:
            fh.writelines([
                "[Desktop Entry]\n",
                "Type=Application\n",
                f"Icon={self.icon_path}\n",
                f"Name={self.APPLICATION_NAME}\n",
                f"Exec={self.application_path}\n",
            ])
            fh.flush()

        # Mark the .desktop file executable like the original script
        os.chmod(self.path, os.stat(self.path).st_mode | stat.S_IEXEC)

        logging.info(self.path)
        logging.info(self.icon_path)

        # Update KDE app cache
        subprocess.check_call([f"kbuildsycoca{KDE_VERSION}"],
                              stdout=sys.stderr, stderr=sys.stderr, env=os.environ)

    def __enter__(self) -> Self:
        self.create()
        return self

    def __exit__(self, *args) -> None:
        # Clean up the created files
        try:
            os.remove(self.path)
        except FileNotFoundError:
            pass
        try:
            os.remove(self.icon_path)
        except FileNotFoundError:
            pass


class TestWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("Test Window")

        self.button = QPushButton("Install Software", self)
        self.button.clicked.connect(self.on_button_clicked)
        self.setCentralWidget(self.button)

        print("Window shown", file=sys.stderr)
        QTimer.singleShot(120_000, self.close)

    def on_button_clicked(self) -> None:
        self.close()


def main() -> None:
    print("Launched", file=sys.stderr)
    app = QApplication(sys.argv)

    # (Optional) You can set desktop file name / app name if desired
    # QApplication.setDesktopFileName(DesktopFileWrapper.APPLICATION_ID)
    app.setApplicationName(DesktopFileWrapper.APPLICATION_NAME)

    print("Activated", file=sys.stderr)
    win = TestWindow()
    win.show()

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
