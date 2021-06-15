/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SPELLCHECKINGSKELETON_H
#define SPELLCHECKINGSKELETON_H

#include <KConfigCore/KCoreConfigSkeleton>

namespace Sonnet
{
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
