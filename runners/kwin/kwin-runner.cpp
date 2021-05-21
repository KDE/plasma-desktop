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
static const QString s_keyword = QStringLiteral("KWin");

KWinRunner::KWinRunner(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
    , m_enabled(false)
{
    setObjectName(s_keyword);
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(s_kwinService, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
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
            addSyntax(Plasma::RunnerSyntax(s_keyword, i18n("Opens the KWin (Plasma Window Manager) debug console.")));
        } else {
            setSyntaxes(QList<Plasma::RunnerSyntax>());
        }
    }
}

#include "kwin-runner.moc"
