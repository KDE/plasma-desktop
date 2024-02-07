#!/usr/bin/env python3

# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

import gi
gi.require_version("GLib", "2.0")
from gi.repository import Gio

def _sort_by_initial_preference(app: Gio.AppInfo) -> int:
    initial_preference = app.get_string('InitialPreference')
    if initial_preference:
        return int(initial_preference)
    return 100

mime_types = {}
for app in Gio.AppInfo.get_all():
    if not (app.get_id().startswith("org.kde.") or app.get_id().startswith("kde-") or app.get_id().startswith("okular")):
        continue
    for type in app.get_supported_types():
        if type in mime_types:
            mime_types[type].append(app)
        else:
            mime_types[type] = [app]

with open('kde-mimeapps.list', 'w') as f:
    f.write('# SPDX-License-Identifier: CC0-1.0\n')
    f.write('# SPDX-FileCopyrightText: None\n')
    f.write("[Default Applications]\n")

    for type in mime_types:
        apps = mime_types[type]
        apps = sorted(apps, key = _sort_by_initial_preference)
        app_ids = [app.get_id() for app in apps]
        f.write(f'{type}={";".join(app_ids)}\n')
