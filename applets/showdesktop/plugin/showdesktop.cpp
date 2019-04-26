/*
 * Copyright 2008  Petri Damsten <damu@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "showdesktop.h"

#include <KWindowSystem>

ShowDesktop::ShowDesktop(QObject *parent) : QObject(parent)
{
    connect(KWindowSystem::self(), &KWindowSystem::showingDesktopChanged,
            this, &ShowDesktop::showingDesktopChanged);
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
