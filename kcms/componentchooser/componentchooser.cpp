/*
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooser.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QMimeDatabase>

#include <KApplicationTrader>
#include <KBuildSycocaProgressDialog>
#include <KConfigGroup>
#include <KOpenWithDialog>
#include <KQuickConfigModule>
#include <KService>
#include <KSharedConfig>

extern KSERVICE_EXPORT int ksycoca_ms_between_checks;

ComponentChooser::ComponentChooser(QObject *parent,
                                   const QString &mimeType,
                                   const QString &applicationCategory,
                                   const QString &defaultApplication,
                                   const QString &dialogText)
    : QObject(parent)
    , m_mimeType(mimeType)
    , m_applicationCategory(applicationCategory)
    , m_defaultApplication(defaultApplication)
    , m_dialogText(dialogText)
{
    qRegisterMetaType<QList<PairQml>>("QList<PairQml>");

    m_model = new ApplicationModel(this);
    connect(m_model, &QAbstractItemModel::modelReset, this, &ComponentChooser::modelChanged);
}

void ComponentChooser::defaults()
{
    const auto defaultIndex = m_model->defaultIndex();
    if (defaultIndex) {
        select(*defaultIndex);
    }
}

void ComponentChooser::load()
{
    m_model->load(m_mimeType, m_applicationCategory, m_defaultApplication, KApplicationTrader::preferredService(m_mimeType));

    m_index = m_model->currentIndex();

    m_currentApplication = currentStorageId();

    Q_EMIT indexChanged();
    Q_EMIT isDefaultsChanged();
}

void ComponentChooser::select(int index)
{
    // Other selection
    if (index == m_model->rowCount() - 1) {
        KOpenWithDialog *dialog = new KOpenWithDialog(QList<QUrl>(), m_mimeType, m_dialogText, QString(), QApplication::activeWindow());
        dialog->setSaveNewApplications(true);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &KOpenWithDialog::finished, this, [this, dialog](int result) {
            if (result == QDialog::Rejected) {
                Q_EMIT indexChanged();
                Q_EMIT isDefaultsChanged();
                return;
            }

            const auto serviceStorageId = dialog->service()->storageId();

            // Check if the selected application is already in the list
            QModelIndex modelIndex = m_model->findByStorageId(serviceStorageId);
            if (modelIndex.isValid()) {
                select(modelIndex.row());
                return;
            }

            auto newIndex = m_model->addApplicationBeforeLast(dialog->service());
            select(newIndex);
        });
        dialog->open();
    } else {
        m_index = index;

        const auto selectedIndex = m_model->index(index, 0);
        m_model->setData(selectedIndex, true, ApplicationModel::Selected);

        Q_EMIT indexChanged();
        Q_EMIT isDefaultsChanged();
        Q_EMIT modelChanged();
    }
}

void ComponentChooser::saveMimeTypeAssociations(const QString &storageId, const QStringList &mimeTypes, bool forceUnsupportedMimeType)
{
    if (storageId.isEmpty()) {
        return;
    }

    // This grabs the configuration from mimeapps.list, which is DE agnostic and part of the XDG standard.
    KSharedConfig::Ptr mimeAppsList = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

    if (mimeAppsList->isConfigWritable(true)) {
        const auto appService = KService::serviceByStorageId(storageId);

        for (const QString &mimeType : mimeTypes) {
            if (!forceUnsupportedMimeType && appService && !serviceSupportsMimeType(appService, mimeType)) {
                // skip mimetype association if the app does not support it at all
                continue;
            }

            KApplicationTrader::setPreferredService(mimeType, appService);
        }

        m_currentApplication = storageId;
    }
}

void ComponentChooser::onSaved()
{
    Q_EMIT indexChanged();
    Q_EMIT isDefaultsChanged();
}

QStringList ComponentChooser::unsupportedMimeTypes() const
{
    const auto preferredApp = currentStorageId();

    if (preferredApp.isEmpty()) {
        return QStringList{};
    }

    const auto db = QMimeDatabase();

    QStringList unsupportedMimeTypes;

    const auto appService = KService::serviceByStorageId(preferredApp);
    const auto componentMimeTypes = mimeTypes();
    for (const QString &mimeType : componentMimeTypes) {
        if (!serviceSupportsMimeType(appService, mimeType)) {
            const auto preferredService = KApplicationTrader::preferredService(mimeType);
            if (!preferredService || (preferredService->storageId() != appService->storageId())) {
                unsupportedMimeTypes << mimeType;
            }
        }
    }

    return unsupportedMimeTypes;
}

void ComponentChooser::saveAssociationUnsuportedMimeTypes()
{
    const auto storageId = currentStorageId();

    saveMimeTypeAssociations(storageId, unsupportedMimeTypes(), true);
    forceReloadServiceCache();
    load();
}

QList<PairQml> ComponentChooser::mimeTypesNotAssociated() const
{
    auto ret = QList<PairQml>();

    const auto db = QMimeDatabase();
    const auto storageId = m_currentApplication;

    const auto appService = KService::serviceByStorageId(storageId);

    if (!appService) {
        return ret;
    }

    const auto mimes = mimeTypes();

    for (const QString &mimeType : mimes) {
        const auto service = KApplicationTrader::preferredService(mimeType);

        if (service && service->storageId() != storageId &&
            // only explicitly supported mimetype
            serviceSupportsMimeType(appService, mimeType)) {
            ret.append(PairQml(service->name(), mimeType));
        }
    }

    return ret;
}

void ComponentChooser::saveMimeTypesNotAssociated()
{
    const auto mimeTypeNotAssociated = mimeTypesNotAssociated();

    auto mimeTypeList = QList<QString>();
    for (const auto &pair : mimeTypeNotAssociated) {
        mimeTypeList.append(pair.second.toString());
    }

    const auto storageId = currentStorageId();
    saveMimeTypeAssociations(storageId, mimeTypeList);

    forceReloadServiceCache();
    load();
}

QString ComponentChooser::currentStorageId() const
{
    return m_model->data(m_index, ApplicationModel::StorageId).toString();
}

QString ComponentChooser::applicationName() const
{
    return m_model->data(m_index, Qt::DisplayRole).toString();
}

QString ComponentChooser::applicationIcon() const
{
    return m_model->data(m_index, ApplicationModel::Icon).toString();
}

QStringList ComponentChooser::mimeTypes() const
{
    return QStringList{};
}

void ComponentChooser::forceReloadServiceCache()
{
    KBuildSycocaProgressDialog::rebuildKSycoca(QApplication::activeWindow());

    // HACK to ensure mime cache is updated right away
    int previous_delay = ksycoca_ms_between_checks;
    ksycoca_ms_between_checks = 0;
    KService::allServices();
    ksycoca_ms_between_checks = previous_delay;
}

void ComponentChooser::save()
{
    // default impl for simple application kinds
    const auto storageId = currentStorageId();

    saveMimeTypeAssociations(storageId, mimeTypes());
}

bool ComponentChooser::isDefaults() const
{
    const auto defaultIndex = m_model->defaultIndex();
    return !defaultIndex.has_value() || *defaultIndex == m_index;
}

bool ComponentChooser::isSaveNeeded() const
{
    const auto storageId = currentStorageId();
    return m_model->rowCount() > 1 && (m_currentApplication != storageId) && storageId != "";
}

bool ComponentChooser::serviceSupportsMimeType(KService::Ptr service, const QString &targetMimeType)
{
    static QMimeDatabase db;
    const QStringList mimeTypes = service->mimeTypes();
    for (const QString &supportedMimeType : mimeTypes) {
        if (supportedMimeType == targetMimeType) {
            return true;
        }

        if (db.mimeTypeForName(targetMimeType).inherits(supportedMimeType)) {
            return true;
        }
    }

    if (targetMimeType.startsWith(QLatin1String("x-scheme-handler/")) && service->schemeHandlers().contains(targetMimeType.mid(17))) {
        return true;
    }

    return false;
}

#include "moc_componentchooser.cpp"
