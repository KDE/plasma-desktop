/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>

class QKeyEvent;

// Simple QGuiApp overlay to gobble up all key events for input-detection.
class Application final : public QGuiApplication
{
    Q_OBJECT
public:
    using QGuiApplication::QGuiApplication;

    bool notify(QObject *receiver, QEvent *event) override;

    static Application *instance();

Q_SIGNALS:
    void keyEvent(QKeyEvent *event);
};

#endif // APPLICATION_H
