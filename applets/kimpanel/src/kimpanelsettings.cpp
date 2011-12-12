/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kimpanelsettings.h"

// KDE
#include <KDirWatch>
#include <KStandardDirs>
#include <KGlobal>

class SettingsHelper
{
public:
    SettingsHelper() : q(0) {}
    // q is also static, no need to delete it here
    ~SettingsHelper() {}
    KimpanelSettings *q;
};

K_GLOBAL_STATIC(SettingsHelper, s_globalSettings)

KimpanelSettings *KimpanelSettings::self()
{
    if (!s_globalSettings->q) {
        s_globalSettings->q = new KimpanelSettings;
        s_globalSettings->q->readConfig();
    }

    return s_globalSettings->q;
}

KimpanelSettings::KimpanelSettings()
{
    //FIXME: if/when kconfig gets change notification, this will be unnecessary
    KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", "kimpanelrc"));
    connect(KDirWatch::self(), SIGNAL(created(QString)), this, SLOT(settingsFileChanged()));
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(settingsFileChanged()));
}

KimpanelSettings::~KimpanelSettings()
{
}

void KimpanelSettings::settingsFileChanged()
{
    readConfig();
    emit configChanged();
}

