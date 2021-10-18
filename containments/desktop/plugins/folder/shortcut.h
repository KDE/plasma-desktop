/*
    SPDX-FileCopyrightText: Ken <https://stackoverflow.com/users/1568857/ken>
    SPDX-FileCopyrightText: 2016 Leslie Zhai <xiangzhai83@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>

/**
 * TODO: ShortCut is a stopgap solution and should be dropped when Qt's StandardKey
 * gains support for these actions. QTBUG-54926 https://bugreports.qt.io/browse/QTBUG-54926
 * And it is *NOT* encouraged registering C++ types with the QML by using EventFilter
 * but for special case QTBUG-40327 https://bugreports.qt.io/browse/QTBUG-40327
 *
 * ShortCut was copied from Ken's answer.
 * https://stackoverflow.com/questions/12192780/assigning-keyboard-shortcuts-to-qml-components
 * it uses cc by-sa 3.0 license by default compatible with GPL.
 * https://www.gnu.org/licenses/license-list.en.html#ccbysa
 */
class ShortCut : public QObject
{
    Q_OBJECT

public:
    explicit ShortCut(QObject *parent = nullptr);

    Q_INVOKABLE void installAsEventFilterFor(QObject *target = nullptr);

Q_SIGNALS:
    void deleteFile();
    void renameFile();
    void moveToTrash();
    void createFolder();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;
};

#endif // SHORTCUT_H
