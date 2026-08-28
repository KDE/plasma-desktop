/*
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KQuickConfigModule>

class SpellCheckingData;

namespace Sonnet
{
class Settings;
}

class SonnetSpellCheckingModule : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(Sonnet::Settings *settings READ settings CONSTANT)

public:
    SonnetSpellCheckingModule(QObject *parent, const KPluginMetaData &data);

    void save() override;
    void defaults() override;
    void load() override;

    Sonnet::Settings *settings() const;

private:
    SpellCheckingData *m_data;
};
