/*
 *   Copyright (C) 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasma-desktop-runner.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>

#include <QDebug>

#include <KLocalizedString>

K_EXPORT_PLASMA_RUNNER_WITH_JSON(PlasmaDesktopRunner, "plasma-runner-plasma-desktop.json")

static const QString s_plasmaService = QLatin1String("org.kde.plasmashell");

PlasmaDesktopRunner::PlasmaDesktopRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args),
      m_desktopConsoleKeyword(i18nc("Note this is a KRunner keyword", "desktop console")),
      m_kwinConsoleKeyword(i18nc("Note this is a KRunner keyword", "wm console")),
      m_enabled(false)
{
    setObjectName( QLatin1String("Plasma-Desktop" ));
    setIgnoredTypes(Plasma::RunnerContext::FileSystem |
                    Plasma::RunnerContext::NetworkLocation |
                    Plasma::RunnerContext::Help);
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_plasmaService, QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, &PlasmaDesktopRunner::checkAvailability);
    checkAvailability(QString(), QString(), QString());
}

PlasmaDesktopRunner::~PlasmaDesktopRunner()
{
}

void PlasmaDesktopRunner::match(Plasma::RunnerContext &context)
{
    if (m_enabled && context.query().startsWith(m_desktopConsoleKeyword, Qt::CaseInsensitive)) {
        Plasma::QueryMatch match(this);
        match.setId(QStringLiteral("plasma-desktop-console"));
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIconName(QStringLiteral("plasma"));
        match.setText(i18n("Open Plasma desktop interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
    if (m_enabled && context.query().startsWith(m_kwinConsoleKeyword, Qt::CaseInsensitive)) {
        Plasma::QueryMatch match(this);
        match.setId(QStringLiteral("plasma-desktop-console"));
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIconName(QStringLiteral("kwin"));
        match.setText(i18n("Open KWin interactive console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
}

void PlasmaDesktopRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)

    if (m_enabled) {
        QDBusMessage message;

        QString query = context.query();
        if (query.compare(m_desktopConsoleKeyword, Qt::CaseInsensitive) == 0) {
            message = QDBusMessage::createMethodCall(s_plasmaService, QStringLiteral("/PlasmaShell"),
                                                     QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("showInteractiveConsole"));
        } else if (query.startsWith(m_desktopConsoleKeyword)) {
            message = QDBusMessage::createMethodCall(s_plasmaService, QStringLiteral("/PlasmaShell"),
                                                     QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("loadScriptInInteractiveConsole"));
            query.replace(m_desktopConsoleKeyword, QString(), Qt::CaseInsensitive);
            QList<QVariant> args;
            args << query;
            message.setArguments(args);
        }
        if (query.compare(m_kwinConsoleKeyword, Qt::CaseInsensitive) == 0) {
            message = QDBusMessage::createMethodCall(s_plasmaService, QStringLiteral("/PlasmaShell"),
                                                     QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("showInteractiveKWinConsole"));
        } else if (query.startsWith(m_kwinConsoleKeyword)) {
            message = QDBusMessage::createMethodCall(s_plasmaService, QStringLiteral("/PlasmaShell"),
                                                     QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("loadKWinScriptInInteractiveConsole"));
            query.replace(m_kwinConsoleKeyword, QString(), Qt::CaseInsensitive);
            QList<QVariant> args;
            args << query;
            message.setArguments(args);
        }

        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void PlasmaDesktopRunner::checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    bool enabled = false;
    if (name.isEmpty()) {
        enabled = QDBusConnection::sessionBus().interface()->isServiceRegistered(s_plasmaService).value();
    } else {
        enabled = !newOwner.isEmpty();
    }

    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (m_enabled) {
            addSyntax(Plasma::RunnerSyntax(m_desktopConsoleKeyword,
                                           i18n("Opens the Plasma desktop interactive console "
                                                "with a file path to a script on disk.")));
            addSyntax(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "desktop console :q:"),
                                           i18n("Opens the Plasma desktop interactive console "
                                                "with a file path to a script on disk.")));
            addSyntax(Plasma::RunnerSyntax(m_kwinConsoleKeyword,
                                           i18n("Opens the KWin interactive console "
                                                "with a file path to a script on disk.")));
            addSyntax(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "wm console :q:"),
                                           i18n("Opens the KWin interactive console "
                                                "with a file path to a script on disk.")));
        } else {
            setSyntaxes(QList<Plasma::RunnerSyntax>());
        }
    }
}

#include "plasma-desktop-runner.moc"

