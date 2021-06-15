/*
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SONNETSPELLCHECKINGMODULE_H
#define SONNETSPELLCHECKINGMODULE_H

#include "spellcheckingskeleton.h"
#include <KCModule>

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
