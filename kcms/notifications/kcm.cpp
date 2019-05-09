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
#include <KLocalizedString>
#include <KNotifyConfigWidget>
#include <KPluginFactory>

#include <algorithm>

#include "sourcesmodel.h"
#include "filterproxymodel.h"

#include <notificationmanager/settings.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMNotificationsFactory, "kcm_notifications.json", registerPlugin<KCMNotifications>();)

KCMNotifications::KCMNotifications(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_sourcesModel(new SourcesModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_settings(new NotificationManager::Settings(this))
{

    const char uri[] = "org.kde.private.kcms.notifications";
    qmlRegisterUncreatableType<SourcesModel>(uri, 1, 0, "SourcesModel",
                                             QStringLiteral("Cannot create instances of SourcesModel"));
    qmlRegisterType<FilterProxyModel>();
    qmlProtectModule(uri, 1);

    KAboutData *about = new KAboutData(QStringLiteral("kcm_notifications"), i18n("Notifications"),
                                       QStringLiteral("5.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@privat.broulik.de"));
    setAboutData(about);

    m_filteredModel->setSourceModel(m_sourcesModel);

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

NotificationManager::Settings *KCMNotifications::settings() const
{
    return m_settings;
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

void KCMNotifications::load()
{
    m_settings->load();
}

void KCMNotifications::save()
{
    m_settings->save();
}

void KCMNotifications::defaults()
{
    m_settings->defaults();
}

#include "kcm.moc"
