/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KConfig>
#include <KConfigGroup>

#include <Plasma/Package>
#include <KQuickAddons/ConfigModule>

class QStandardItemModel;

class KCMSplashScreen : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel *splashModel READ splashModel CONSTANT)
    Q_PROPERTY(QString selectedPlugin READ selectedPlugin WRITE setSelectedPlugin NOTIFY selectedPluginChanged)
    Q_PROPERTY(int selectedPluginIndex READ selectedPluginIndex NOTIFY selectedPluginIndexChanged)
    Q_PROPERTY(bool testing READ testing NOTIFY testingChanged)

public:
    enum Roles {
        PluginNameRole = Qt::UserRole +1,
        ScreenhotRole,
        DescriptionRole
    };
    KCMSplashScreen(QObject* parent, const QVariantList& args);

    QList<Plasma::Package> availablePackages(const QString &component);

    QStandardItemModel *splashModel();

    QString selectedPlugin() const;
    void setSelectedPlugin(const QString &plugin);

    int selectedPluginIndex() const;

    bool testing() const;

    void loadModel();

public Q_SLOTS:
    void getNewClicked();
    void load() override;
    void save() override;
    void defaults() override;
    void test(const QString &plugin);

Q_SIGNALS:
    void selectedPluginChanged();
    void selectedPluginIndexChanged();

    void testingChanged();
    void testingFailed();

private:
    QStandardItemModel *m_model;
    Plasma::Package m_package;
    QString m_selectedPlugin;

    QProcess *m_testProcess = nullptr;

    KConfig m_config;
    KConfigGroup m_configGroup;
};

#endif
