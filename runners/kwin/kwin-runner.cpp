/*
    SPDX-FileCopyrightText: 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kwin-runner.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(KWinRunner, "plasma-runner-kwin.json")

static const QString s_kwinService = QStringLiteral("org.kde.KWin");
static const QString s_keyword = QStringLiteral("kwin debug console");
static const QStringList s_keywords = {QStringLiteral("kwin"), QStringLiteral("debug"), QStringLiteral("console")};

KWinRunner::KWinRunner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_kwinService, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KWinRunner::checkAvailability);
    checkAvailability(QString(), QString(), QString());
}

void KWinRunner::match(RunnerContext &context)
{
    const bool containsAnyKeyword = std::any_of(s_keywords.begin(), s_keywords.end(), [&context](const QString &keyword) {
        return context.query().contains(keyword, Qt::CaseInsensitive);
    });
    if (m_enabled && containsAnyKeyword) {
        QueryMatch match(this);
        match.setId(QStringLiteral("kwin"));
        if (context.query().compare(s_keyword, Qt::CaseInsensitive) == 0) {
            match.setCategoryRelevance(QueryMatch::CategoryRelevance::Highest);
        }
        match.setIconName(QStringLiteral("kwin"));
        match.setText(i18n("Open KWin debug console"));
        match.setRelevance(1.0);
        context.addMatch(match);
    }
}

void KWinRunner::run(const RunnerContext & /*context*/, const QueryMatch & /*match*/)
{
    if (m_enabled) {
        QDBusMessage message = QDBusMessage::createMethodCall(s_kwinService, QStringLiteral("/KWin"), s_kwinService, QStringLiteral("showDebugConsole"));
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void KWinRunner::checkAvailability(const QString &name, const QString & /*oldOwner*/, const QString &newOwner)
{
    bool enabled = false;
    if (name.isEmpty()) {
        enabled = QDBusConnection::sessionBus().interface()->isServiceRegistered(s_kwinService).value();
    } else {
        enabled = !newOwner.isEmpty();
    }

    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (m_enabled) {
            RunnerSyntax(s_keywords, i18n("Opens the KWin (Plasma Window Manager) debug console."));
        } else {
            setSyntaxes(QList<RunnerSyntax>());
        }
    }
}

#include "kwin-runner.moc"

#include "moc_kwin-runner.cpp"
