/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "doodad.h"

#include <QKeyEvent>
#include <QX11Info>

#include "application.h"
#include "shape.h"

Doodad *Doodad::factorize(XkbDoodadPtr doodad, XkbDescPtr xkb, QObject *parent)
{
    switch (doodad->any.type) {
    case XkbOutlineDoodad:
    case XkbSolidDoodad:
        return new ShapeDoodad(doodad, xkb, parent);
    case XkbTextDoodad:
        return new TextDoodad(doodad, xkb, parent);
    case XkbIndicatorDoodad:
        return new IndicatorDoodad(doodad, xkb, parent);
    case XkbLogoDoodad:
        return new LogoDoodad(doodad, xkb, parent);
    }

    Q_UNREACHABLE();
    return nullptr;
}

Doodad::Doodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent)
    : XkbObject(xkb_, parent)
    , doodad(doodad_)
{
}

ShapeDoodad::ShapeDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent)
    : Doodad(doodad_, xkb_, parent)
    , shape(new Shape(xkb->geom->shapes + doodad_->shape.shape_ndx, xkb, this))
    , outlineOnly(doodad->any.type == XkbOutlineDoodad)
{
}

TextDoodad::TextDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent)
    : Doodad(doodad_, xkb_, parent)
{
}

IndicatorDoodad::IndicatorDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent)
    : Doodad(doodad_, xkb_, parent)
    , shape(new Shape(xkb->geom->shapes + doodad_->indicator.shape_ndx, xkb, this))
{
    connect(Application::instance(), &Application::keyEvent, this, &IndicatorDoodad::refreshState);
    refreshState();
}

void IndicatorDoodad::refreshState()
{
    int onInt = False;
    if (!XkbGetNamedIndicator(QX11Info::display(), doodad->indicator.name, nullptr, &onInt, nullptr, nullptr)) {
        on = false;
    } else {
        on = onInt == True ? true : false;
    }
    Q_EMIT onChanged();
}

LogoDoodad::LogoDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent)
    : ShapeDoodad(doodad_, xkb_, parent)
{
}
