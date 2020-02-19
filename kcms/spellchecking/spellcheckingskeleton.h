/*

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

#ifndef SPELLCHECKINGSKELETON_H
#define SPELLCHECKINGSKELETON_H

#include <KConfigCore/KCoreConfigSkeleton>

namespace Sonnet {
    class Settings;
    class ConfigView;
}

class SpellCheckingSkeleton : public KCoreConfigSkeleton
{
    Q_OBJECT

public:
    explicit SpellCheckingSkeleton(QObject *parent = nullptr);
    bool usrSave() override;
    void usrRead() override;

    void setPreferredLanguages(const QStringList &preferredLanguages);
    QStringList preferredLanguages() const;

    void setIgnoreList(const QStringList &ignoreList);
    QStringList ignoreList() const;

    void setDefaultLanguage(const QString &defaultLanguage);
    QString defaultLanguage() const;

    QStringList clients() const;
private:
    Sonnet::Settings *m_store;
    QStringList m_preferredLanguages;
    QStringList m_ignoreList;
    QString m_defaultLanguage;
};

#endif
