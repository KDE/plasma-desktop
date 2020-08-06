/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kcm.h"

#include <QAction>
#include <QCommandLineParser>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QWindow>

#include <KAboutData>
#include <KConfigGroup>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KNotifyConfigWidget>
#include <KPluginFactory>

#include <algorithm>

#include "sourcesmodel.h"
#include "filterproxymodel.h"

#include <notificationmanager/donotdisturbsettings.h>
#include <notificationmanager/notificationsettings.h>
#include <notificationmanager/jobsettings.h>
#include <notificationmanager/badgesettings.h>
#include <notificationmanager/behaviorsettings.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMNotificationsFactory, "kcm_notifications.json", registerPlugin<KCMNotifications>();)

KCMNotifications::KCMNotifications(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_sourcesModel(new SourcesModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_dndSettings(new NotificationManager::DoNotDisturbSettings(this))
    , m_notificationSettings(new NotificationManager::NotificationSettings(this))
    , m_jobSettings(new NotificationManager::JobSettings(this))
    , m_badgeSettings(new NotificationManager::BadgeSettings(this))
    , m_toggleDoNotDisturbAction(new QAction(this))
{

    const char uri[] = "org.kde.private.kcms.notifications";
    qmlRegisterUncreatableType<SourcesModel>(uri, 1, 0, "SourcesModel",
                                             QStringLiteral("Cannot create instances of SourcesModel"));

    qmlRegisterType<FilterProxyModel>();
    qmlRegisterType<QKeySequence>();
    qmlRegisterType<NotificationManager::DoNotDisturbSettings>();
    qmlRegisterType<NotificationManager::NotificationSettings>();
    qmlRegisterType<NotificationManager::JobSettings>();
    qmlRegisterType<NotificationManager::BadgeSettings>();
    qmlRegisterType<NotificationManager::BehaviorSettings>();
    qmlProtectModule(uri, 1);

    KAboutData *about = new KAboutData(QStringLiteral("kcm_notifications"), i18n("Notifications"),
                                       QStringLiteral("5.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@privat.broulik.de"));
    setAboutData(about);

    m_filteredModel->setSourceModel(m_sourcesModel);

    // for KGlobalAccel...
    // keep in sync with globalshortcuts.cpp in notification plasmoid!
    m_toggleDoNotDisturbAction->setObjectName(QStringLiteral("toggle do not disturb"));
    m_toggleDoNotDisturbAction->setProperty("componentName", QStringLiteral("plasmashell"));
    m_toggleDoNotDisturbAction->setText(i18n("Toggle do not disturb"));
    m_toggleDoNotDisturbAction->setIcon(QIcon::fromTheme(QStringLiteral("notifications-disabled")));

    QStringList stringArgs;
    stringArgs.reserve(args.count() + 1);
    // need to add a fake argv[0] for QCommandLineParser
    stringArgs.append(QStringLiteral("kcm_notifications"));
    for (const QVariant &arg : args) {
        stringArgs.append(arg.toString());
    }

    QCommandLineParser parser;

    QCommandLineOption desktopEntryOption(QStringLiteral("desktop-entry"), QString(), QStringLiteral("desktop-entry"));
    parser.addOption(desktopEntryOption);
    QCommandLineOption notifyRcNameOption(QStringLiteral("notifyrc"), QString(), QStringLiteral("notifyrcname"));
    parser.addOption(notifyRcNameOption);
    QCommandLineOption eventIdOption(QStringLiteral("event-id"), QString(), QStringLiteral("event-id"));
    parser.addOption(eventIdOption);

    parser.parse(stringArgs);

    setInitialDesktopEntry(parser.value(desktopEntryOption));
    setInitialNotifyRcName(parser.value(notifyRcNameOption));
    setInitialEventId(parser.value(eventIdOption));

    connect(this, &KCMNotifications::toggleDoNotDisturbShortcutChanged, this, &KCMNotifications::settingsChanged);
}

KCMNotifications::~KCMNotifications()
{

}

SourcesModel *KCMNotifications::sourcesModel() const
{
    return m_sourcesModel;
}

FilterProxyModel *KCMNotifications::filteredModel() const
{
    return m_filteredModel;
}

NotificationManager::DoNotDisturbSettings *KCMNotifications::dndSettings() const
{
    return m_dndSettings;
}

NotificationManager::NotificationSettings *KCMNotifications::notificationSettings() const
{
    return m_notificationSettings;
}

NotificationManager::JobSettings *KCMNotifications::jobSettings() const
{
    return m_jobSettings;
}

NotificationManager::BadgeSettings *KCMNotifications::badgeSettings() const
{
    return m_badgeSettings;
}

QKeySequence KCMNotifications::toggleDoNotDisturbShortcut() const
{
    return m_toggleDoNotDisturbShortcut;
}

void KCMNotifications::setToggleDoNotDisturbShortcut(const QKeySequence &shortcut)
{
    if (m_toggleDoNotDisturbShortcut == shortcut) {
        return;
    }

    m_toggleDoNotDisturbShortcut = shortcut;
    m_toggleDoNotDisturbShortcutDirty = true;
    emit toggleDoNotDisturbShortcutChanged();
}

QString KCMNotifications::initialDesktopEntry() const
{
    return m_initialDesktopEntry;
}

void KCMNotifications::setInitialDesktopEntry(const QString &desktopEntry)
{
    if (m_initialDesktopEntry != desktopEntry) {
        m_initialDesktopEntry = desktopEntry;
        emit initialDesktopEntryChanged();
    }
}

QString KCMNotifications::initialNotifyRcName() const
{
    return m_initialNotifyRcName;
}

void KCMNotifications::setInitialNotifyRcName(const QString &notifyRcName)
{
    if (m_initialNotifyRcName != notifyRcName) {
        m_initialNotifyRcName = notifyRcName;
        emit initialNotifyRcNameChanged();
    }
}

QString KCMNotifications::initialEventId() const
{
    return m_initialEventId;
}

void KCMNotifications::setInitialEventId(const QString &eventId)
{
    if (m_initialEventId != eventId) {
        m_initialEventId = eventId;
        emit initialEventIdChanged();
    }
}

void KCMNotifications::configureEvents(const QString &notifyRcName, const QString &eventId, QQuickItem *ctx)
{
    // We're not using KNotifyConfigWidget::configure here as we want to handle the
    // saving ourself (so we Apply with all other KCM settings) but there's no way
    // to access the config object :(
    // We also need access to the QDialog so we can set the KCM as transient parent.

    QDialog *dialog = new QDialog(nullptr);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(i18n("Configure Notifications"));

    if (ctx && ctx->window()) {
        dialog->winId(); // so it creates windowHandle
        dialog->windowHandle()->setTransientParent(QQuickRenderControl::renderWindowFor(ctx->window()));
        dialog->setModal(true);
    }

    KNotifyConfigWidget *w = new KNotifyConfigWidget(dialog);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(w);
    layout->addWidget(buttonBox);
    dialog->setLayout(layout);

    // TODO we should only save settings when clicking Apply in the main UI
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, w, &KNotifyConfigWidget::save);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, w, &KNotifyConfigWidget::save);
    connect(w, &KNotifyConfigWidget::changed, buttonBox->button(QDialogButtonBox::Apply), &QPushButton::setEnabled);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    w->setApplication(notifyRcName);
    w->selectEvent(eventId);

    dialog->show();
}

NotificationManager::BehaviorSettings *KCMNotifications::behaviorSettings(const QModelIndex &index)
{
    if (!index.isValid()) {
        return nullptr;
    }
    return m_behaviorSettingsList.value(index.row());
}

void KCMNotifications::load()
{
    ManagedConfigModule::load();

    bool firstLoad = m_firstLoad;
    if (m_firstLoad) {
        m_firstLoad = false;
        m_sourcesModel->load();

        for (int i = 0; i < m_sourcesModel->rowCount(); ++i) {
            const QModelIndex index = m_sourcesModel->index(i, 0);
            if (!index.isValid()) {
                continue;
            }

            QString typeName;
            QString groupName;
            if (m_sourcesModel->data(index, SourcesModel::SourceTypeRole) == SourcesModel::ApplicationType) {
                typeName = QStringLiteral("Applications");
                groupName = m_sourcesModel->data(index, SourcesModel::DesktopEntryRole).toString();
            } else {
                typeName = QStringLiteral("Services");
                groupName = m_sourcesModel->data(index, SourcesModel::NotifyRcNameRole).toString();
            }
            auto *toAdd = new NotificationManager::BehaviorSettings(typeName, groupName, this);
            m_behaviorSettingsList[index.row()] = toAdd;
            createConnections(toAdd);
        }
    }

    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->load();
    }

    const QKeySequence toggleDoNotDisturbShortcut = KGlobalAccel::self()->globalShortcut(
        m_toggleDoNotDisturbAction->property("componentName").toString(),
        m_toggleDoNotDisturbAction->objectName()).value(0);

    if (m_toggleDoNotDisturbShortcut != toggleDoNotDisturbShortcut) {
        m_toggleDoNotDisturbShortcut = toggleDoNotDisturbShortcut;
        emit toggleDoNotDisturbShortcutChanged();
    }

    m_toggleDoNotDisturbShortcutDirty = false;
    if (firstLoad) {
        emit firstLoadDone();
    }
}

void KCMNotifications::save()
{
    ManagedConfigModule::save();
    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->save();
    }

    if (m_toggleDoNotDisturbShortcutDirty) {
        // KeySequenceItem will already have checked whether the shortcut is available
        KGlobalAccel::self()->setShortcut(m_toggleDoNotDisturbAction,
                                          {m_toggleDoNotDisturbShortcut},
                                          KGlobalAccel::NoAutoloading);
    }
}

void KCMNotifications::defaults()
{
    ManagedConfigModule::defaults();
    for (auto *behaviorSettings : qAsConst(m_behaviorSettingsList)) {
        behaviorSettings->setDefaults();
    }

    setToggleDoNotDisturbShortcut(QKeySequence());
}

bool KCMNotifications::isSaveNeeded() const
{
    bool needSave = std::any_of(m_behaviorSettingsList.cbegin(),
                                m_behaviorSettingsList.cend(),
                                [](const NotificationManager::BehaviorSettings *settings) {
                                    return settings->isSaveNeeded();
                                });

    return needSave || m_toggleDoNotDisturbShortcutDirty;
}

bool KCMNotifications::isDefaults() const
{
    bool notDefault = std::any_of(m_behaviorSettingsList.cbegin(),
                                  m_behaviorSettingsList.cend(),
                                  [](const NotificationManager::BehaviorSettings *settings) {
                                      return !settings->isDefaults();
                                  });
    return !notDefault;
}

void KCMNotifications::createConnections(NotificationManager::BehaviorSettings *settings)
{
    connect(settings, &NotificationManager::BehaviorSettings::ShowPopupsChanged, this, &KCMNotifications::settingsChanged);
    connect(settings, &NotificationManager::BehaviorSettings::ShowPopupsInDndModeChanged, this, &KCMNotifications::settingsChanged);
    connect(settings, &NotificationManager::BehaviorSettings::ShowInHistoryChanged, this, &KCMNotifications::settingsChanged);
    connect(settings, &NotificationManager::BehaviorSettings::ShowBadgesChanged, this, &KCMNotifications::settingsChanged);
}

#include "kcm.moc"
