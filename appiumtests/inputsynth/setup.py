#!/usr/bin/env python3
"""
SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
SPDX-License-Identifier: GPL-2.0-or-later
"""

import importlib
import os
import shutil

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):

    def __init__(self, name, cmake_lists_dir='.', **kwa) -> None:
        Extension.__init__(self, name, sources=[], **kwa)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class cmake_build_ext(build_ext):

    def build_extensions(self) -> None:
        suffix: str = importlib.machinery.EXTENSION_SUFFIXES[0]
        for ext in self.extensions:
            extdir: str = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            if not os.path.exists(extdir):
                os.makedirs(extdir)
            shutil.copy2(os.path.join(os.getcwd(), f"{ext.name}.so"), os.path.join(extdir, f"{ext.name}{suffix}"))


setup(
    name="inputsynth_plasma_desktop",
    version="0.1",
    ext_modules=[CMakeExtension("inputsynth_plasma_desktop")],
    cmdclass={'build_ext': cmake_build_ext},
)
