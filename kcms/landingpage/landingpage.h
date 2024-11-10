/*
    SPDX-FileCopyrightText: 2018 <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KPackage/Package>
#include <KQuickManagedConfigModule>

#include <QAction>
#include <QJsonArray>
#include <QJsonValue>
#include <QSortFilterProxyModel>

class QStandardItemModel;

class LandingPageData;
class LandingPageGlobalsSettings;

namespace KActivities
{
namespace Stats
{
class ResultModel;
}
}

class MostUsedModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum Roles {
        KcmPluginRole = Qt::UserRole + 1000,
    };

    explicit MostUsedModel(QObject *parent = nullptr);
    void setResultModel(KActivities::Stats::ResultModel *model);
    QHash<int, QByteArray> roleNames() const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    // Model when there is nothing from kactivities-stat
    QStandardItemModel *m_defaultModel = nullptr;
    // Model fed by kactivities-stats
    KActivities::Stats::ResultModel *m_resultModel = nullptr;
    mutable QStringList ignoredKCMs;
};

class LookAndFeelGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QUrl thumbnail READ thumbnail CONSTANT)

public:
    LookAndFeelGroup(QObject *parent = nullptr);
    QString id() const;
    QString name() const;
    QUrl thumbnail() const;

    KPackage::Package m_package;
};

class KCMLandingPage : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(MostUsedModel *mostUsedModel READ mostUsedModel CONSTANT)
    Q_PROPERTY(LandingPageGlobalsSettings *globalsSettings READ globalsSettings CONSTANT)
    Q_PROPERTY(LookAndFeelGroup *defaultLightLookAndFeel READ defaultLightLookAndFeel CONSTANT)
    Q_PROPERTY(LookAndFeelGroup *defaultDarkLookAndFeel READ defaultDarkLookAndFeel CONSTANT)

public:
    KCMLandingPage(QObject *parent, const KPluginMetaData &metaData);
    ~KCMLandingPage() override
    {
    }

    MostUsedModel *mostUsedModel() const;

    LandingPageGlobalsSettings *globalsSettings() const;

    LookAndFeelGroup *defaultLightLookAndFeel() const;
    LookAndFeelGroup *defaultDarkLookAndFeel() const;

    Q_INVOKABLE void openKCM(const QString &kcm);

    Q_INVOKABLE QAction *kcmAction(const QString &storageId);

public Q_SLOTS:
    void save() override;

private:
    LandingPageData *m_data = nullptr;

    LookAndFeelGroup *m_defaultLightLookAndFeel = nullptr;
    LookAndFeelGroup *m_defaultDarkLookAndFeel = nullptr;

    MostUsedModel *m_mostUsedModel = nullptr;

    bool m_lnfDirty = false;
};
