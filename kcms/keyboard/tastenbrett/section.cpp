/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "section.h"

#include <QDebug>

#include "doodad.h"
#include "row.h"

Section::Section(XkbSectionPtr section_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , section(section_)
{
    for (int i = 0; i < section->num_doodads; ++i) {
        Doodad *o = Doodad::factorize(section->doodads + i, xkb, this);
        if (!o) {
            continue;
        }
        doodads.push_back(o);
    }

    for (int i = 0; i < section->num_rows; ++i) {
        rows.push_back(new Row(section->rows + i, xkb, this));
    }

    // Sections also contain overlays, that contain overlay rows, that contain
    // overlay keys, that are comprised of an "under" name (internal name
    // of our Key object) and an "over" name (the overlay to be used instead).
    // The intention I presume is to label keys this way. Alas, this seems
    // useless to us because that'd ignore key mapping. Our dynamic key cap
    // resolution should be yielding more useful cap data than these overlays.
    // Because of this we do not actually set our caps to the overlays and ignore
    // them entirely.
}
