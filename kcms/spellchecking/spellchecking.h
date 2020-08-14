/*

Copyright 2008 Albert Astals Cid <aacid@kde.org>
Copyright 2020 Benjamin Port <benjamin.port@enioka.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SONNETSPELLCHECKINGMODULE_H
#define SONNETSPELLCHECKINGMODULE_H

#include <KCModule>
#include "spellcheckingskeleton.h"


class KConfigDialogManager;
class SpellCheckingData;
class SpellCheckingSkeleton;

namespace Sonnet
{
    class ConfigView;
    class Settings;
}

class SonnetSpellCheckingModule : public KCModule
{
    Q_OBJECT

public:
    SonnetSpellCheckingModule(QWidget *parent, const QVariantList &);
    ~SonnetSpellCheckingModule() override;

    void save() override;
    void load() override;
    void defaults() override;

    SpellCheckingSkeleton *skeleton() const;

private:
    void stateChanged();

    Sonnet::Settings *m_settings;
    Sonnet::ConfigView *m_configWidget;
    SpellCheckingData *m_data;
    KConfigDialogManager *m_managedConfig;
};

#endif
