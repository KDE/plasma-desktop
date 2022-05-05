/*
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooser.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include <KApplicationTrader>
#include <KConfigGroup>
#include <KOpenWithDialog>
#include <KQuickAddons/ConfigModule>
#include <KService>
#include <KSharedConfig>

ComponentChooser::ComponentChooser(QObject *parent,
                                   const QString &mimeType,
                                   const QString &category,
                                   const QString &defaultApplication,
                                   const QString &dialogText)
    : QObject(parent)
    , m_mimeType(mimeType)
    , m_category(category)
    , m_defaultApplication(defaultApplication)
    , m_dialogText(dialogText)
{
}

void ComponentChooser::defaults()
{
    if (m_defaultIndex) {
        select(*m_defaultIndex);
    }
}

void ComponentChooser::load()
{
    // This unregisters the QVariantList of applications and frees resources.
    m_applications.clear();

    bool preferredServiceAdded = false;

    /* KService::Ptr is a shared data pointer for a KService (a.k.a. an application).
     * KApplicationTrader::preferredService returns the preferred application for a specific mimetype.
     * Internally it just runs KApplicationTrader::queryByMimeType,
     * which lists the applications associated with a mimetype,
     * then grabs its first index, that is, [0].
     */
    KService::Ptr preferredService = KApplicationTrader::preferredService(m_mimeType);

    /* This function is for populating the list of applications.
     * KApplicationTrader::query asks for the list of applications that match a mimetype
     * according to a selected filter made with KApplicationTrader::FilterFunc,
     * which in turn is just std::function<bool(const KService::Ptr &)>.
     * Here the filter is a lambda that requires:
     * - Confirmation that the preferred service has been added
     * - Knowledge of the preferred application (a.k.a. at index [0])
     * - Defining which application will be set up
     * Else, if the .desktop file is invalid in any way, fail immediately.
     * A service type in this context is a URI handler or a mimetype.
     */
    KApplicationTrader::query([&preferredServiceAdded, preferredService, this](const KService::Ptr &service) {
        if (service->exec().isEmpty() || (!m_category.isEmpty() && !service->categories().contains(m_category))
            || (!service->serviceTypes().contains(m_mimeType))) {
            return false;
        }
        QVariantMap application;
        application[QStringLiteral("name")] = service->name();
        application[QStringLiteral("icon")] = service->icon();
        // storageId = org.kde.myapplication.desktop
        application[QStringLiteral("storageId")] = service->storageId();
        // We then add this mapped object to our list of applications.
        m_applications += application;
        /* Here we evaluate
         * if there is a preferred application AND the preferred application is the one we just passed
         * then we set its index to the number of applications listed minus one.
         */
        if ((preferredService && preferredService->storageId() == service->storageId())) {
            m_index = m_applications.length() - 1;
            preferredServiceAdded = true;
        }
        /* Instead, if the application we pass is already a default application,
         * we set our default index to be the number of applications minus one
         * because it is an array that starts from 0.
         */
        if (service->storageId() == m_defaultApplication) {
            m_defaultIndex = m_applications.length() - 1;
        }
        return false;
    });
    /* If a preferred application already exists and no preferred application
     * (as defined by our above code) was added with this query,
     * then it means that the user is manually setting something as default.
     * We act accordingly and pass that application to the list
     * by using a mapped object.
     */
    if (preferredService && !preferredServiceAdded) {
        QVariantMap application;
        application["name"] = preferredService->name();
        application["icon"] = preferredService->icon();
        application["storageId"] = preferredService->storageId();
        m_applications += application;
        m_index = m_applications.length() - 1;
    }
    /* This adds an unlabeled default application (Other...),
     * which can be set to anything and is executed with a shell.
     * It is at the bottom of the ComboBox list.
     */
    QVariantMap application;
    application["name"] = i18n("Other…");
    application["icon"] = QStringLiteral("application-x-shellscript");
    application["storageId"] = QString();
    m_applications += application;
    if (m_index == -1) {
        m_index = 0;
    }
    // This is needed to determine whether a change was made,
    // in which case we need to sync and reparse the changes.
    m_previousApplication = m_applications[m_index].toMap()["storageId"].toString();
    Q_EMIT applicationsChanged();
    Q_EMIT indexChanged();
    Q_EMIT isDefaultsChanged();
}

// This is for the case the user clicks in Other...
void ComponentChooser::select(int index)
{
    // If the current index is the same as the selected one
    // and there's more than one item in the list, proceed.
    if (m_index == index && m_applications.size() != 1) {
        return;
    }
    if (index == m_applications.length() - 1) {
        KOpenWithDialog *dialog = new KOpenWithDialog(QList<QUrl>(), m_mimeType, m_dialogText, QString());
        dialog->setSaveNewApplications(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &KOpenWithDialog::finished, this, [this, dialog](int result) {
            if (result == QDialog::Rejected) {
                Q_EMIT indexChanged();
                Q_EMIT isDefaultsChanged();
                return;
            }

            const KService::Ptr service = dialog->service();
            // Check if the selected application is already in the list,
            // if it is, set it as a default application.
            for (int i = 0; i < m_applications.length(); i++) {
                if (m_applications[i].toMap()["storageId"] == service->storageId()) {
                    m_index = i;
                    Q_EMIT indexChanged();
                    Q_EMIT isDefaultsChanged();
                    return;
                }
            }
            // If the application has an icon, set it,
            // otherwise fallback to generic icon.
            const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
            // Now that the default application selection window has finished,
            // add its data to the list of applications.
            QVariantMap application;
            application["name"] = service->name();
            application["icon"] = icon;
            application["storageId"] = service->storageId();
            application["execLine"] = service->exec();
            m_applications.insert(m_applications.length() - 1, application);
            m_index = m_applications.length() - 2;
            Q_EMIT applicationsChanged();
            Q_EMIT indexChanged();
            Q_EMIT isDefaultsChanged();
        });
        dialog->open();
    } else {
        m_index = index;
    }
    Q_EMIT indexChanged();
    Q_EMIT isDefaultsChanged();
}

void ComponentChooser::saveMimeTypeAssociations(const QStringList &mimes, const QString &storageId)
{
    // This grabs the configuration from mimeapps.list, which is DE agnostic and part of the XDG standard.
    KSharedConfig::Ptr mimeAppsList = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

    if (mimeAppsList->isConfigWritable(true)) {
        /* KConfigGroup creates objects that refer to the groups
         * [Default Applications] and [Added Associations]
         * which are found in mimeapps.list and used for the
         * --group flag of kwriteconfig5 for instance.
         */
        KConfigGroup defaultApps(mimeAppsList, QStringLiteral("Default Applications"));
        KConfigGroup addedApps(mimeAppsList, QStringLiteral("Added Associations"));
        /* In the ComponentChooser child classes for each association, we add a QStringList
         * containing all mimetypes we want to associate to a certain application.
         * We pass this as `mimes` here and iterate through it
         * so that we can set associations in bulk.
         */
        for (const QString &m : mimes) {
            // We grab each mimetype and storage id and pass it to our default apps list.
            defaultApps.writeXdgListEntry(m, QStringList(storageId));

            // We make a list of applications relative to a given mimetype,
            // then make it the first in the list and sync.
            QStringList apps = addedApps.readXdgListEntry(m);
            apps.removeAll(storageId);
            apps.prepend(storageId);
            addedApps.writeXdgListEntry(m, apps);
        }
        m_previousApplication = m_applications[m_index].toMap()["storageId"].toString();
        // And we finish by notifying that the changes are done.
        mimeAppsList->sync();
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                              QStringLiteral("/KLauncher"),
                                                              QStringLiteral("org.kde.KLauncher"),
                                                              QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
    }
}

bool ComponentChooser::isDefaults() const
{
    // We check whether the default index is assigned to anything or whether the index is a default.
    return !m_defaultIndex.has_value() || *m_defaultIndex == m_index;
}

bool ComponentChooser::isSaveNeeded() const
{
    /* If the list of applications is NOT empty
     * AND the previously set application is different from the current index,
     * then a change was made.
     */
    return !m_applications.isEmpty() && (m_previousApplication != m_applications[m_index].toMap()["storageId"].toString());
}