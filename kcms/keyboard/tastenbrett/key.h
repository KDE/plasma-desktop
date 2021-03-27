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

#ifndef KEY_H
#define KEY_H

#include "xkbobject.h"

class Shape;

// This is a fairly opinionated model of a key cap. We assume there won't
// be any more than 4 levels.
// This isn't necessarily the case since (e.g.) the NEO layout has 6 levels.
// Since that is super niche and poses some complication as for how to
// exactly render the levels we'll simply ignore this use case for now.
// In the end the keyboard model display only needs to act as simple indicator
// anyway.
class KeyCap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString topLeft MEMBER topLeft CONSTANT)
    Q_PROPERTY(QString topRight MEMBER topRight CONSTANT)
    Q_PROPERTY(QString bottomLeft MEMBER bottomLeft CONSTANT)
    Q_PROPERTY(QString bottomRight MEMBER bottomRight CONSTANT)
public:
    enum Level {
        TopLeft = 1,
        TopRight = 3,
        BottomLeft = 0,
        BottomRight = 2,
    };
    Q_ENUM(Level)
    constexpr static int levelCount = 4;

    KeyCap(const QString symbols[KeyCap::levelCount], QObject *parent);

    QString topLeft;
    QString topRight;
    QString bottomLeft;
    QString bottomRight;
};

class Key : public XkbObject
{
    Q_OBJECT
#define K_P(type, name)                                                                                                                                        \
private:                                                                                                                                                       \
    Q_PROPERTY(type name READ auto_prop_##name CONSTANT)                                                                                                       \
public:                                                                                                                                                        \
    type auto_prop_##name() const                                                                                                                              \
    {                                                                                                                                                          \
        return key->name;                                                                                                                                      \
    }

    K_P(short, gap)

    Q_PROPERTY(Shape *shape MEMBER shape CONSTANT)
    Q_PROPERTY(KeyCap *cap MEMBER cap CONSTANT)
    Q_PROPERTY(bool pressed MEMBER pressed NOTIFY pressedChanged)

    constexpr static uint INVALID_KEYCODE = static_cast<uint>(-1);

public:
    Key(XkbKeyPtr key_, XkbDescPtr xkb_, QObject *parent = nullptr);
    uint nativeScanCodeFromName(const QByteArray &needle);
    KeyCap *resolveCap();

    XkbKeyPtr key = nullptr;
    Shape *shape = nullptr;
    QByteArray name; // Internal name in the geometry.
    quint32 nativeScanCode = INVALID_KEYCODE;
    KeyCap *cap = nullptr;
    bool pressed = false;

Q_SIGNALS:
    void pressedChanged();
};

#endif // KEY_H
