/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KNSCore/EntryWrapper>
#include <KPackage/Package>
#include <KQuickAddons/ManagedConfigModule>

class QStandardItemModel;
class QSortFilterProxyModel;
class SplashScreenSettings;
class SplashScreenData;

class KCMSplashScreen : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(SplashScreenSettings *splashScreenSettings READ splashScreenSettings CONSTANT)
    Q_PROPERTY(QAbstractProxyModel *splashSortedModel READ splashSortedModel CONSTANT)
    Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)

public:
    enum Roles {
        PluginNameRole = Qt::UserRole + 1,
        ScreenshotRole,
        DescriptionRole,
        UninstallableRole,
        PendingDeletionRole,
    };

    KCMSplashScreen(QObject *parent, const QVariantList &args);

    SplashScreenSettings *splashScreenSettings() const;
    QAbstractProxyModel *splashSortedModel() const;
    bool testing() const;

    Q_INVOKABLE int sortModelPluginIndex(const QString &pluginName) const;
    Q_INVOKABLE void ghnsEntryChanged(KNSCore::EntryWrapper *wrapper);
    QStringList pendingDeletions();

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;
    void test(const QString &plugin);

Q_SIGNALS:
    void testingChanged();
    void testingFailed(const QString &processErrorOutput);
    void error(const QString &message);

private:
    QList<KPackage::Package> availablePackages(const QString &component);
    int pluginIndex(const QString &pluginName) const;
    void addKPackageToModel(const KPackage::Package &pkg);

    SplashScreenData *m_data;
    QStandardItemModel *m_model;

    QProcess *m_testProcess = nullptr;
    QString m_packageRoot;
    QSortFilterProxyModel *m_sortModel;
};

#endif
