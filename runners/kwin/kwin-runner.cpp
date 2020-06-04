/*
 *   Copyright (C) 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>
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

#include "kwin-runner.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusConnectionInterface>

#include <KLocalizedString>

K_EXPORT_PLASMA_RUNNER_WITH_JSON(KWinRunner, "plasma-runner-kwin.json")

static const QString s_kwinService = QStringLiteral("org.kde.KWin");
static const QString s_keyword = QStringLiteral("KWin");

KWinRunner::KWinRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args),
      m_enabled(false)
{
    setObjectName(s_keyword);
    setIgnoredTypes(Plasma::RunnerContext::FileSystem |
                    Plasma::RunnerContext::NetworkLocation |
                    Plasma::RunnerContext::Help);
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_kwinService, QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KWinRunner::checkAvailability);
    checkAvailability(QString(), QString(), QString());
}

KWinRunner::~KWinRunner()
{
}

void KWinRunner::match(Plasma::RunnerContext &context)
{
    if (m_enabled && context.query().compare(s_keyword, Qt::CaseInsensitive) == 0) {
        Plasma::QueryMatch match(this);
        match.setId(QStringLiteral("kwin"));
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIconName(QStringLiteral("kwin"));
        match.setText(i18n("Open KWin debug console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
}

void KWinRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)

    if (m_enabled && context.query().compare(s_keyword, Qt::CaseInsensitive) == 0) {
        QDBusMessage message = QDBusMessage::createMethodCall(s_kwinService, QStringLiteral("/KWin"), s_kwinService, QStringLiteral("showDebugConsole"));
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void KWinRunner::checkAvailability(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    bool enabled = false;
    if (name.isEmpty()) {
        enabled = QDBusConnection::sessionBus().interface()->isServiceRegistered(s_kwinService).value();
    } else {
        enabled = !newOwner.isEmpty();
    }

    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (m_enabled) {
            addSyntax(Plasma::RunnerSyntax(s_keyword,
                                           i18n("Opens the KWin (Plasma Window Manager) debug console.")));
        } else {
            setSyntaxes(QList<Plasma::RunnerSyntax>());
        }
    }
}

#include "kwin-runner.moc"
