/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TASTENBRETT_H
#define TASTENBRETT_H

#include <QString>

class Tastenbrett
{
public:
    static QString path();
    static bool exists();
    static void launch(const QString &model, const QString &layout, const QString &variant, const QString &options, const QString &title = QString());
};

#endif // TASTENBRETT_H
