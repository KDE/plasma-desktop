/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DOODAD_H
#define DOODAD_H

#include <QColor>

#include "xkbobject.h"

class Shape;

class Doodad : public XkbObject
{
    Q_OBJECT

#define D_P(type, name)                                                                                                                                        \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return doodad->any.name;                                                                                                                               \
    }

    D_P(unsigned char, priority)

    static Doodad *factorize(XkbDoodadPtr doodad, XkbDescPtr xkb, QObject *parent);

    XkbDoodadPtr doodad = nullptr;

protected:
    Doodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent = nullptr);
};

class ShapeDoodad : public Doodad
{
    Q_OBJECT

#define SD_P(type, name)                                                                                                                                       \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return doodad->shape.name;                                                                                                                             \
    }

    SD_P(short, top)
    SD_P(short, left)
    SD_P(short, angle)

    Q_PROPERTY(Shape *shape MEMBER shape CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
    // Whether this shape is an outline only. If it is not it's solid/filled.
    Q_PROPERTY(bool outlineOnly MEMBER outlineOnly CONSTANT)

public:
    ShapeDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent = nullptr);

    Shape *shape = nullptr;
    QColor color;
    bool outlineOnly = false;
};

class TextDoodad : public Doodad
{
    Q_OBJECT

#define TD_P(type, name)                                                                                                                                       \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return doodad->text.name;                                                                                                                              \
    }

    TD_P(short, top)
    TD_P(short, left)
    TD_P(short, angle)
    TD_P(short, width)
    TD_P(short, height)
    TD_P(QString, text)
    TD_P(QString, font)
public:
    TextDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent = nullptr);

    Shape *shape = nullptr;
};

// NB: This is technically kind of like a shape doodad, but in reality
//    only top/left/angle are actually equal across doodad.indicator. and doodad.shape.
//    As such there is no benefit in modelling the classes like there is inheritance
//    because there really isn't. The actual shape definition is abstracted by
//    Shape objects in either case.
//    On the GUI side this can still be rendered like a shape, since it has a Shape*,
//    it's just not specifically a shape doodad.
class IndicatorDoodad : public Doodad
{
    Q_OBJECT

#define ID_P(type, name)                                                                                                                                       \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return doodad->indicator.name;                                                                                                                         \
    }

    ID_P(short, top)
    ID_P(short, left)
    ID_P(short, angle)

    Q_PROPERTY(Shape *shape MEMBER shape CONSTANT)
    Q_PROPERTY(bool on MEMBER on NOTIFY onChanged)
public:
    IndicatorDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent = nullptr);

    Shape *shape = nullptr;
    bool on = false;

Q_SIGNALS:
    void onChanged();

private Q_SLOTS:
    void refreshState();
};

class LogoDoodad : public ShapeDoodad
{
    Q_OBJECT

#define LD_P(type, name)                                                                                                                                       \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return doodad->logo.name;                                                                                                                              \
    }

    LD_P(short, top)
    LD_P(short, left)
    LD_P(short, angle)
    LD_P(QString, logo_name)
public:
    LogoDoodad(XkbDoodadPtr doodad_, XkbDescPtr xkb_, QObject *parent = nullptr);

    Shape *shape = nullptr;
};

#endif // DOODAD_H
