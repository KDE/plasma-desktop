/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_keys.h"

#include <QDBusMetaType>
#include <QFile>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QWindow>

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KMessageBox>
#include <KOpenWithDialog>
#include <KPluginFactory>
#include <kglobalaccel_interface.h>

#include "basemodel.h"
#include "filteredmodel.h"
#include "globalaccelmodel.h"
#include "kcmkeys_debug.h"
#include "keysdata.h"
#include "shortcutsmodel.h"
#include "standardshortcutsmodel.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMKeysFactory, "kcm_keys.json", registerPlugin<KCMKeys>(); registerPlugin<KeysData>();)

KCMKeys::KCMKeys(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{
    constexpr char uri[] = "org.kde.private.kcms.keys";
    qmlRegisterUncreatableType<BaseModel>(uri, 2, 0, "BaseModel", "Can't create BaseModel");
    qmlRegisterAnonymousType<ShortcutsModel>(uri, 2);
    qmlRegisterAnonymousType<FilteredShortcutsModel>(uri, 2);
    qmlProtectModule(uri, 2);
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
    qDBusRegisterMetaType<QList<QStringList>>();
    KAboutData *about = new KAboutData(QStringLiteral("kcm_keys"), i18n("Shortcuts"), QStringLiteral("2.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("David Redondo"), QString(), QStringLiteral("kde@david-redondo.de"));
    setAboutData(about);
    m_globalAccelInterface = new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"), //
                                                       QStringLiteral("/kglobalaccel"),
                                                       QDBusConnection::sessionBus(),
                                                       this);
    if (!m_globalAccelInterface->isValid()) {
        setError(i18n("Failed to communicate with global shortcuts daemon"));
        qCCritical(KCMKEYS) << "Interface is not valid";
        if (m_globalAccelInterface->lastError().isValid()) {
            qCCritical(KCMKEYS) << m_globalAccelInterface->lastError().name() << m_globalAccelInterface->lastError().message();
        }
    }
    m_globalAccelModel = new GlobalAccelModel(m_globalAccelInterface, this);
    m_standardShortcutsModel = new StandardShortcutsModel(this);
    m_shortcutsModel = new ShortcutsModel(this);
    m_shortcutsModel->addSourceModel(m_globalAccelModel);
    m_shortcutsModel->addSourceModel(m_standardShortcutsModel);
    m_filteredModel = new FilteredShortcutsModel(this);
    m_filteredModel->setSourceModel(m_shortcutsModel);

    connect(m_shortcutsModel, &QAbstractItemModel::dataChanged, this, [this] {
        setNeedsSave(m_globalAccelModel->needsSave() || m_standardShortcutsModel->needsSave());
        setRepresentsDefaults(m_globalAccelModel->isDefault() && m_standardShortcutsModel->isDefault());
    });
    connect(m_shortcutsModel, &QAbstractItemModel::modelReset, this, [this] {
        setNeedsSave(false);
        setRepresentsDefaults(m_globalAccelModel->isDefault() && m_standardShortcutsModel->isDefault());
    });

    connect(m_globalAccelModel, &GlobalAccelModel::errorOccured, this, &KCMKeys::setError);
}

void KCMKeys::load()
{
    m_globalAccelModel->load();
    m_standardShortcutsModel->load();
}

void KCMKeys::save()
{
    m_globalAccelModel->save();
    m_standardShortcutsModel->save();
}

void KCMKeys::defaults()
{
    m_globalAccelModel->defaults();
    m_standardShortcutsModel->defaults();
}

ShortcutsModel *KCMKeys::shortcutsModel() const
{
    return m_shortcutsModel;
}

FilteredShortcutsModel *KCMKeys::filteredModel() const
{
    return m_filteredModel;
}

void KCMKeys::setError(const QString &errorMessage)
{
    m_lastError = errorMessage;
    Q_EMIT this->errorOccured();
}

QString KCMKeys::lastError() const
{
    return m_lastError;
}

void KCMKeys::writeScheme(const QUrl &url)
{
    qCDebug(KCMKEYS) << "Exporting to " << url.toLocalFile();
    KConfig file(url.toLocalFile(), KConfig::SimpleConfig);
    m_globalAccelModel->exportToConfig(file);
    m_standardShortcutsModel->exportToConfig(file);
    file.sync();
}

void KCMKeys::loadScheme(const QUrl &url)
{
    qCDebug(KCMKEYS) << "Loading scheme" << url.toLocalFile();
    KConfig file(url.toLocalFile(), KConfig::SimpleConfig);
    m_globalAccelModel->importConfig(file);
    m_standardShortcutsModel->importConfig(file);
}

QVariantList KCMKeys::defaultSchemes() const
{
    QVariantList schemes;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kcmkeys"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.kksrc"));
        for (const QString &file : fileNames) {
            const QString path = dir + QLatin1Char('/') + file;
            KConfig scheme(path, KConfig::SimpleConfig);
            const QString name = KConfigGroup(&scheme, "Settings").readEntry("Name", file);
            schemes.append(QVariantMap({{"name", name}, {"url", QUrl::fromLocalFile(path)}}));
        }
    }
    return schemes;
}

void KCMKeys::addApplication(QQuickItem *ctx)
{
    auto dialog = new KOpenWithDialog;
    if (ctx && ctx->window()) {
        dialog->winId(); // so it creates windowHandle
        dialog->windowHandle()->setTransientParent(QQuickRenderControl::renderWindowFor(ctx->window()));
        dialog->setWindowModality(Qt::WindowModal);
    }
    dialog->hideRunInTerminal();
    dialog->setSaveNewApplications(true);
    dialog->open();
    connect(dialog, &KOpenWithDialog::finished, this, [this, dialog](int result) {
        if (result == QDialog::Accepted && dialog->service()) {
            const KService::Ptr service = dialog->service();
            const QString desktopFileName = service->storageId();
            if (m_globalAccelModel->match(m_shortcutsModel->index(0, 0), BaseModel::ComponentRole, desktopFileName, 1, Qt::MatchExactly).isEmpty()) {
                m_globalAccelModel->addApplication(desktopFileName, service->name());
            } else {
                qCDebug(KCMKEYS) << "Already have component" << service->storageId();
            }
        }
        dialog->deleteLater();
    });
}

QString KCMKeys::keySequenceToString(const QKeySequence &keySequence) const
{
    return keySequence.toString(QKeySequence::NativeText);
}

QString KCMKeys::urlFilename(const QUrl &url)
{
    return url.fileName();
}

QModelIndex KCMKeys::conflictingIndex(const QKeySequence &keySequence)
{
    for (int i = 0; i < m_shortcutsModel->rowCount(); ++i) {
        const QModelIndex componentIndex = m_shortcutsModel->index(i, 0);
        for (int j = 0; j < m_shortcutsModel->rowCount(componentIndex); ++j) {
            const QModelIndex actionIndex = m_shortcutsModel->index(j, 0, componentIndex);
            if (m_shortcutsModel->data(actionIndex, BaseModel::ActiveShortcutsRole).value<QSet<QKeySequence>>().contains(keySequence)) {
                return m_shortcutsModel->mapToSource(actionIndex);
            }
        }
    }
    return QModelIndex();
}

void KCMKeys::requestKeySequence(QQuickItem *context, const QModelIndex &index, const QKeySequence &newSequence, const QKeySequence &oldSequence)
{
    qCDebug(KCMKEYS) << index << "wants" << newSequence << "instead of" << oldSequence;
    const QModelIndex conflict = conflictingIndex(newSequence);
    if (!conflict.isValid()) {
        auto model = const_cast<BaseModel *>(static_cast<const BaseModel *>(index.model()));
        if (!oldSequence.isEmpty()) {
            model->changeShortcut(index, oldSequence, newSequence);
        } else {
            model->addShortcut(index, newSequence);
        }
        return;
    }

    qCDebug(KCMKEYS) << "Found conflict for" << newSequence << conflict;
    const bool isStandardAction = conflict.parent().data(BaseModel::SectionRole).toString() == i18n("Common Actions");
    const QString actionName = conflict.data().toString();
    const QString componentName = conflict.parent().data().toString();
    const QString keysString = newSequence.toString(QKeySequence::NativeText);
    const QString message = isStandardAction
        ? i18nc("%2 is the name of a category inside the 'Common Actions' section",
                "Shortcut %1 is already assigned to the common %2 action '%3'.\nDo you want to reassign it?",
                keysString,
                componentName,
                actionName)
        : i18n("Shortcut %1 is already assigned to action '%2' of %3.\nDo you want to reassign it?", keysString, actionName, componentName);
    const QString title = i18nc("@title:window", "Found conflict");
    auto dialog = new QDialog;
    dialog->setWindowTitle(title);
    if (context && context->window()) {
        dialog->winId(); // so it creates windowHandle
        dialog->windowHandle()->setTransientParent(QQuickRenderControl::renderWindowFor(context->window()));
    }
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    KMessageBox::createKMessageBox(dialog,
                                   new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, dialog),
                                   QMessageBox::Question,
                                   message,
                                   {},
                                   QString(),
                                   nullptr,
                                   KMessageBox::NoExec);
    dialog->show();

    connect(dialog, &QDialog::finished, this, [index, conflict, newSequence, oldSequence](int result) {
        auto model = const_cast<BaseModel *>(static_cast<const BaseModel *>(index.model()));
        if (result != QDialogButtonBox::Yes) {
            // Also Q_EMIT if we are not changing anything, to force the frontend to update and be consistent
            // with the model. It is currently out of sync because it reflects the user input that
            // was rejected now.
            Q_EMIT model->dataChanged(index, index, {BaseModel::ActiveShortcutsRole, BaseModel::CustomShortcutsRole});
            return;
        }
        const_cast<BaseModel *>(static_cast<const BaseModel *>(conflict.model()))->disableShortcut(conflict, newSequence);
        if (!oldSequence.isEmpty()) {
            model->changeShortcut(index, oldSequence, newSequence);
        } else {
            model->addShortcut(index, newSequence);
        }
    });
}

#include "kcm_keys.moc"
