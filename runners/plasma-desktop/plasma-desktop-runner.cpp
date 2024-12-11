/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "plasma-desktop-runner.h"

#include <KIO/CommandLauncherJob>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

#include <QDebug>

#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(PlasmaDesktopRunner, "plasma-runner-plasma-desktop.json")

static const QString s_plasmaService = QLatin1String("org.kde.plasmashell");

PlasmaDesktopRunner::PlasmaDesktopRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
    , m_desktopConsoleKeyword(i18nc("Note this is a KRunner keyword", "desktop console"))
    , m_kwinConsoleKeyword(i18nc("Note this is a KRunner keyword", "wm console"))
{
    setObjectName(QLatin1String("Plasma-Desktop"));
    addSyntax(m_desktopConsoleKeyword, i18n("Opens the Plasma desktop interactive console with a file path to a script on disk."));
    addSyntax(i18nc("Note this is a KRunner keyword", "desktop console :q:"),
              i18n("Opens the Plasma desktop interactive console with a file path to a script on disk."));
    addSyntax(m_kwinConsoleKeyword, i18n("Opens the KWin interactive console with a file path to a script on disk."));
    addSyntax(i18nc("Note this is a KRunner keyword", "wm console :q:"), i18n("Opens the KWin interactive console with a file path to a script on disk."));
}

void PlasmaDesktopRunner::match(RunnerContext &context)
{
    if (context.query().startsWith(m_desktopConsoleKeyword, Qt::CaseInsensitive)) {
        QueryMatch match(this);
        match.setId(QStringLiteral("plasma-desktop-console"));
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        match.setIconName(QStringLiteral("plasma"));
        match.setText(i18n("Open Plasma desktop interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
    if (context.query().startsWith(m_kwinConsoleKeyword, Qt::CaseInsensitive)) {
        QueryMatch match(this);
        match.setId(QStringLiteral("plasma-desktop-console"));
        match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        match.setIconName(QStringLiteral("kwin"));
        match.setText(i18n("Open KWin interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
}

void PlasmaDesktopRunner::run(const RunnerContext &context, const QueryMatch & /*match*/)
{
    bool showPlasmaConsole = context.query().startsWith(m_desktopConsoleKeyword, Qt::CaseInsensitive);
    QStringList args{showPlasmaConsole ? QStringLiteral("--plasma") : QStringLiteral("--kwin")};
    auto job = new KIO::CommandLauncherJob(QStringLiteral("plasma-interactiveconsole"), args);
    job->start();
}

#include "plasma-desktop-runner.moc"

#include "moc_plasma-desktop-runner.cpp"
