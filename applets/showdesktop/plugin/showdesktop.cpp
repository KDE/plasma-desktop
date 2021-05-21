/*
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "showdesktop.h"

#include <KWindowSystem>

ShowDesktop::ShowDesktop(QObject *parent)
    : QObject(parent)
{
    connect(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged, this, &ShowDesktop::showingDesktopChanged);
}

ShowDesktop::~ShowDesktop() = default;

bool ShowDesktop::showingDesktop() const
{
    return KWindowSystem::showingDesktop();
}

void ShowDesktop::setShowingDesktop(bool showingDesktop)
{
    KWindowSystem::setShowingDesktop(showingDesktop);
}

void ShowDesktop::minimizeAll()
{
    const auto &windows = KWindowSystem::windows();
    for (WId wid : windows) {
        KWindowSystem::minimizeWindow(wid);
    }
}
