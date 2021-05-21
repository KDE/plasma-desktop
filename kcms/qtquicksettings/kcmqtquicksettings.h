/*  This file is part of the KDE's Plasma desktop
    SPDX-FileCopyrightText: 2017 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <KConfigGroup>
#include <KSharedConfig>

namespace Ui
{
class KCMQtQuickSettingsWidget;
}

namespace PlasmaQtQuickSettings
{
class RendererSettings;
}

/**
 * @short A KCM to configure Plasma QtQuick settings
 */
class KCMQtQuickSettingsModule : public KCModule
{
    Q_OBJECT

public:
    explicit KCMQtQuickSettingsModule(QWidget *parent, const QVariantList &);
    ~KCMQtQuickSettingsModule() override;

private:
    QScopedPointer<Ui::KCMQtQuickSettingsWidget> m_ui;
    QScopedPointer<PlasmaQtQuickSettings::RendererSettings> m_settings;
};
