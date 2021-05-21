/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEDACTIONS_H
#define KDEDACTIONS_H

#include <KActionCollection>

class TouchpadGlobalActions : public KActionCollection
{
    Q_OBJECT
public:
    explicit TouchpadGlobalActions(bool isConfiguration, QObject *parent);

Q_SIGNALS:
    void enableTriggered();
    void disableTriggered();
    void toggleTriggered();
};

#endif // KDEDACTIONS_H
