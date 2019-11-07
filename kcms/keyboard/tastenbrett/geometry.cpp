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

#include "geometry.h"

#include <QDebug>

#include "doodad.h"
#include "section.h"

Geometry::Geometry(XkbGeometryPtr geom_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , geom(geom_)
    , widthMM(geom->width_mm)
    , heightMM(geom->height_mm)
{
    for (int i = 0; i < geom->num_doodads; ++i) {
        Doodad *o = Doodad::factorize(geom->doodads + i, xkb, this);
        if (!o) {
            continue;
        }
        doodads.push_back(o);
    }

    for (int i = 0; i < geom->num_sections; ++i) {
        sections.push_back(new Section(geom->sections + i, xkb, this));
    }
}
