/*
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

#include "kwin_interface.h"

class ShowDesktop : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showingDesktop READ showingDesktop WRITE setShowingDesktop NOTIFY showingDesktopChanged)

public:
    explicit ShowDesktop(QObject *parent = nullptr);
    ~ShowDesktop() override;

    bool showingDesktop() const;
    void setShowingDesktop(bool showingDesktop);

    Q_INVOKABLE void toggleDesktop();

Q_SIGNALS:
    void showingDesktopChanged(bool showingDesktop);

private:
    OrgKdeKWinInterface *m_interface;
};
