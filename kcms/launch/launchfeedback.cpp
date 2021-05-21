/*
    SPDX-FileCopyrightText: 2001 Rik Hemsley (rikkus) <rik@kde.org>
    SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "launchfeedback.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include "launchfeedbackdata.h"
#include "launchfeedbacksettings.h"

K_PLUGIN_FACTORY_WITH_JSON(LaunchFactory, "kcm_launchfeedback.json", registerPlugin<LaunchFeedback>(); registerPlugin<LaunchFeedbackData>();)

LaunchFeedback::LaunchFeedback(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LaunchFeedbackData(this))
{
    constexpr char uri[] = "org.kde.private.kcms.launchfeedback";
    qmlRegisterUncreatableType<LaunchFeedback>(uri, 1, 0, "KCM", QStringLiteral("Cannot create instances of KCM"));
    qmlRegisterAnonymousType<LaunchFeedbackSettings>(uri, 1);

    KAboutData *about = new KAboutData(QStringLiteral("kcm_launchfeedback"),
                                       i18n("Configure application launch feedback"),
                                       QStringLiteral("0.2"),
                                       QString(),
                                       KAboutLicense::LGPL);
    setAboutData(about);

    setButtons(Apply | Default);
}

LaunchFeedback::~LaunchFeedback()
{
}

LaunchFeedbackSettings *LaunchFeedback::launchFeedbackSettings() const
{
    return m_data->settings();
}

#include "launchfeedback.moc"
