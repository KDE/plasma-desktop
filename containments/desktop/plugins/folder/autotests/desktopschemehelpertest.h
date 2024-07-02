/*
    SPDX-FileCopyrightText: 2022 Alexander Wilms <f.alexander.wilms@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DESKTOPSCHEMEHELPERTEST_H
#define DESKTOPSCHEMEHELPERTEST_H

#include <QObject>

class DesktopSchemeHelperTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void returnsExpectedValues();
};

#endif // DESKTOPSCHEMEHELPERTEST_H
