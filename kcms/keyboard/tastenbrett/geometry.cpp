/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

Geometry::~Geometry()
{
    XkbFreeKeyboard(xkb, 0, True);
}
