/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
#include <kdirwatch.h>
#include <kstandarddirs.h>
#include <kdebug.h>

namespace KIM
{
    class SettingsHelper
    {
      public:
        SettingsHelper() : q(0) {}
        // q is also static, no need to delete it here
        ~SettingsHelper() {}
        Settings *q;
    };
    K_GLOBAL_STATIC(SettingsHelper, s_globalSettings)
    Settings *Settings::self()
    {
      if (!s_globalSettings->q) {
        s_globalSettings->q = new Settings;
        s_globalSettings->q->readConfig();
      }

      return s_globalSettings->q;
    }

    Settings::Settings()
    {
        //FIXME: if/when kconfig gets change notification, this will be unnecessary
        KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", "kimpanelrc"));
        connect(KDirWatch::self(), SIGNAL(created(const QString &)), this, SLOT(settingsFileChanged()));
        connect(KDirWatch::self(), SIGNAL(dirty(const QString &)), this, SLOT(settingsFileChanged()));
    }

    Settings::~Settings()
    {
    }

    void Settings::settingsFileChanged()
    {
        readConfig();
        emit configChanged();
    }
} // namespace KIM

// vim: sw=4 sts=4 et tw=100
