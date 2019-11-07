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

#include "shape.h"

#include "outline.h"

Shape::Shape(XkbShapePtr shape_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , shape(shape_)
    , bounds(QRect(shape->bounds.x1, shape->bounds.y1,
                   shape->bounds.x2, shape->bounds.y2))
{
    // Awkward hack. We only ever render one outline because
    // otherwise we wouldn't know where to stick labels so they
    // don't overlap with any of the outlines.
    // Also when primary is not set the first outline is the primary
    // outline as per documentation, so effectively we unify behavior
    // here by always ensuring the primary outline is the first outline
    // and that will be the one that is rendered.
    if (shape->primary) {
        outlines.push_back(new Outline(shape->primary, xkb, this));
        return;
    }

    for (int i = 0; i < shape->num_outlines; ++i) {
        outlines.push_back(new Outline(shape->outlines + i, xkb, this));
    }
}
