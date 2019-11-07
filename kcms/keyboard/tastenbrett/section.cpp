/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
