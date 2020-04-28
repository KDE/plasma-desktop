/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

#include "kcm_keys.h"

#include <QDBusMetaType>
#include <QFile>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QWindow>

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <kglobalaccel_interface.h>
#include <KGlobalShortcutInfo>
#include <KLocalizedString>
#include <KMessageBox>
#include <KOpenWithDialog>
#include <KPluginFactory>

#include "filteredmodel.h"
#include "kcmkeys_debug.h"
#include "shortcutsmodel.h"

K_PLUGIN_CLASS_WITH_JSON(KCMKeys, "kcm_keys.json")

KCMKeys::KCMKeys(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{
    constexpr char uri[] = "org.kde.private.kcms.keys";
    qmlRegisterUncreatableType<ShortcutsModel>(uri, 2, 0, "ShortcutsModel", "Can't create ShortcutsModel");
    qmlRegisterAnonymousType<FilteredShortcutsModel>(uri, 2);
    qmlRegisterAnonymousType<KGlobalAccelInterface>(uri, 2);
    qmlProtectModule(uri, 2);
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();
    qDBusRegisterMetaType<QList<QStringList>>();
    KAboutData *about = new KAboutData(QStringLiteral("kcm_keys"), i18n("Global Shortcuts"),
        QStringLiteral("2.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("David Redondo"), QString(), QStringLiteral("kde@david-redondo.de"));
    setAboutData(about);
    m_globalAccelInterface = new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel")
         , QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus(), this);
    if (!m_globalAccelInterface->isValid()) {
        setError(i18n("Failed to communicate with global shortcuts daemon"));
        qCCritical(KCMKEYS) << "Interface is not valid";
        if (m_globalAccelInterface->lastError().isValid()) {
           qCCritical(KCMKEYS) <<  m_globalAccelInterface->lastError().name() << m_globalAccelInterface->lastError().message();
        }
    }
    m_shortcutsModel = new ShortcutsModel(m_globalAccelInterface, this);
    m_fileredModel = new FilteredShortcutsModel(this);
    m_fileredModel->setSourceModel(m_shortcutsModel);
    connect(m_shortcutsModel, &QAbstractItemModel::dataChanged, this, [this] {
        setNeedsSave(m_shortcutsModel->needsSave());
        setRepresentsDefaults(m_shortcutsModel->isDefault());
    });
    connect(m_shortcutsModel, &QAbstractItemModel::modelReset, this, [this] {
        setNeedsSave(false);
        setRepresentsDefaults(m_shortcutsModel->isDefault());
    });

    connect(m_shortcutsModel, &ShortcutsModel::errorOccured, this, &KCMKeys::setError);
}

void KCMKeys::load()
{
    m_shortcutsModel->load();
}

void KCMKeys::save()
{
    m_shortcutsModel->save();
}

void KCMKeys::defaults()
{
    m_shortcutsModel->defaults();
}

ShortcutsModel* KCMKeys::shortcutsModel() const
{
    return m_shortcutsModel;
}

FilteredShortcutsModel* KCMKeys::filteredModel() const
{
    return m_fileredModel;
}

void KCMKeys::setError(const QString &errorMessage)
{
    m_lastError = errorMessage;
    emit this->errorOccured();
}

QString KCMKeys::lastError() const
{
  return m_lastError;
}

void KCMKeys::writeScheme(const QUrl &url)
{
    const auto includedComponents = m_shortcutsModel->match(m_shortcutsModel->index(0, 0), ShortcutsModel::CheckedRole, true, -1);
    qCDebug(KCMKEYS) << "Exporting to " << url.toLocalFile();
    KConfig file(url.toLocalFile(), KConfig::SimpleConfig);
    for (const QModelIndex &componentIndex : includedComponents) {
        KConfigGroup mainGroup(&file, componentIndex.data(ShortcutsModel::ComponentRole).toString());
        KConfigGroup group(&mainGroup, "Global Shortcuts");
        for (int i = 0; i < m_shortcutsModel->rowCount(componentIndex); ++i) {
            const QModelIndex shortcutIndex = m_shortcutsModel->index(i, 0, componentIndex);
            const auto activeShortcuts = shortcutIndex.data(ShortcutsModel::ActiveShortcutsRole).value<QSet<QKeySequence>>();
            const QList<QKeySequence> shortcutsList(activeShortcuts.cbegin(), activeShortcuts.cend());
            group.writeEntry(shortcutIndex.data(ShortcutsModel::ActionRole).toString(), QKeySequence::listToString(shortcutsList));
        }
    }
    file.sync();
}

void KCMKeys::loadScheme(const QUrl &url) 
{
    qCDebug(KCMKEYS) << "Loading scheme" << url.toLocalFile();
    KConfig file(url.toLocalFile(), KConfig::SimpleConfig);
    m_shortcutsModel->setShortcuts(file);
}

QVariantList KCMKeys::defaultSchemes() const
{
    QVariantList schemes;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                            QStringLiteral("kcmkeys"), QStandardPaths::LocateDirectory);
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
    dialog->open();
    connect(dialog, &KOpenWithDialog::finished, this, [this, dialog] (int result) {
        if (result == QDialog::Accepted && dialog->service()) {
            const KService::Ptr service = dialog->service();
            const QString desktopFileName = service->desktopEntryName() + ".desktop";
            if (m_shortcutsModel->match(m_shortcutsModel->index(0, 0), ShortcutsModel::ComponentRole, desktopFileName).isEmpty()) {
                const QString destination = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kglobalaccel/";
                QFile::copy(service->entryPath(), destination + desktopFileName);
                m_shortcutsModel->addApplication(desktopFileName, service->name());
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

#include "kcm_keys.moc"
