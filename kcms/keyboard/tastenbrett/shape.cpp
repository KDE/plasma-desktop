/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "shape.h"

#include "outline.h"

Shape::Shape(XkbShapePtr shape_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , shape(shape_)
    , bounds(QRect(shape->bounds.x1, shape->bounds.y1, shape->bounds.x2, shape->bounds.y2))
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
