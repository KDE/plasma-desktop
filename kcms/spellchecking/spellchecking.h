/*
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KQuickConfigModule>

class SpellCheckingSkeleton;

namespace Sonnet
{
class Settings;
}

class SonnetSpellCheckingModule : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(Sonnet::Settings *settings MEMBER m_settings CONSTANT)

public:
    SonnetSpellCheckingModule(QObject *parent, const KPluginMetaData &data);

    void save() override;
    void defaults() override;

private:
    Sonnet::Settings *m_settings;
};
